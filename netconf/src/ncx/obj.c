/*  FILE: obj.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
09dec07      abb      begun
21jul08      abb      start obj-based rewrite

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

#ifndef _H_dlq
#include "dlq.h"
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

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xpath
#include "xpath.h"
#endif

#ifndef _H_yang
#include "yang.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
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
static obj_case_t * 
    new_case (boolean isreal);

static void
    free_case (obj_case_t *cas);


/********************************************************************
* FUNCTION find_type_in_grpchain
* 
* Search for a defined typedef in the typeQ of each grouping
* in the chain
*
* INPUTS:
*   obj == blank template to free
*
* RETURNS:
*   pointer to found type template, NULL if not found
*********************************************************************/
static typ_template_t *
    find_type_in_grpchain (grp_template_t *grp,
			   const xmlChar *typname)
{
    typ_template_t  *typ;
    grp_template_t  *testgrp;

    typ = ncx_find_type_que(&grp->typedefQ, typname);
    if (typ) {
	return typ;
    }

    testgrp = grp->parentgrp;
    while (testgrp) {
	typ = ncx_find_type_que(&testgrp->typedefQ, typname);
	if (typ) {
	    return typ;
	}
	testgrp = testgrp->parentgrp;
    }
    return NULL;

}  /* find_type_in_grpchain */


/********************************************************************
* FUNCTION new_blank_template
* 
* Malloc and initialize the fields in a an obj_template_t
* Do not malloc or initialize any of the def union pointers
*
*
* RETURNS:
*   pointer to the malloced and partially initialized struct;
*   NULL if an error
*********************************************************************/
static obj_template_t * 
    new_blank_template (void)
{
    obj_template_t  *obj;

    obj = m__getObj(obj_template_t);
    if (!obj) {
	return NULL;
    }
    (void)memset(obj, 0x0, sizeof(obj_template_t));
    dlq_createSQue(&obj->metadataQ);
    dlq_createSQue(&obj->appinfoQ);
    dlq_createSQue(&obj->iffeatureQ);
    return obj;

}  /* new_blank_template */


/********************************************************************
* FUNCTION clean_mustQ
* 
* Clean a Q of xpath_pcb_t entries
*
* INPUTS:
*   mustQ == Q of ncx_errinfo_t to delete
*
*********************************************************************/
static void
    clean_mustQ (dlq_hdr_t *mustQ)
{
    xpath_pcb_t *must;

    while (!dlq_empty(mustQ)) {
	must = (xpath_pcb_t *)dlq_deque(mustQ);
	xpath_free_pcb(must);
    }

}  /* clean_mustQ */


/********************************************************************
* FUNCTION clean_metadataQ
* 
* Clean a Q of obj_metadata_t
*
* INPUTS:
*   metadataQ == Q of obj_metadata_t to delete
*
*********************************************************************/
static void
    clean_metadataQ (dlq_hdr_t *metadataQ)
{
    obj_metadata_t *meta;

    while (!dlq_empty(metadataQ)) {
	meta = (obj_metadata_t *)dlq_deque(metadataQ);
	obj_free_metadata(meta);
    }

}  /* clean_metadataQ */


/********************************************************************
* FUNCTION clone_datadefQ
* 
* Clone a Q of obj_template_t
*
* INPUTS:
*    mod == module owner of the cloned data
*    newQ == Q of obj_template_t getting new contents
*    srcQ == Q of obj_template_t with starting contents
*    mobjQ == Q of OBJ_TYP_REFINE obj_template_t to 
*            merge into the clone, as refinements
*            (May be NULL)
*   parent == parent object containing the srcQ (may be NULL)
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    clone_datadefQ (ncx_module_t *mod,
		    dlq_hdr_t *newQ,
		    dlq_hdr_t *srcQ,
		    dlq_hdr_t *mobjQ,
		    obj_template_t *parent)
{
    obj_template_t  *newobj, *srcobj, *testobj;
    status_t         res;

    res = NO_ERR;

    for (srcobj = (obj_template_t *)dlq_firstEntry(srcQ);
	 srcobj != NULL;
	 srcobj = (obj_template_t *)dlq_nextEntry(srcobj)) {

	if (!obj_has_name(srcobj)) {
	    continue;
	}

	newobj = obj_clone_template(mod, srcobj, mobjQ);
	if (!newobj) {
	    log_error("\nError: clone of object %s failed",
		      obj_get_name(srcobj));
	    return ERR_INTERNAL_MEM;
	} else {
	    testobj = obj_find_template(newQ, 
					obj_get_mod_name(newobj),
					obj_get_name(newobj));
	    if (testobj) {
		log_error("\nError: Object %s on line %s "
			  "already defined at line %u",
			  obj_get_name(newobj),
			  srcobj->tkerr.linenum,
			  testobj->tkerr.linenum);
		obj_free_template(newobj);
	    } else {
		newobj->parent = parent;
		dlq_enque(newobj, newQ);
	    }
	}
    }

    return res;

}  /* clone_datadefQ */


/********************************************************************
* FUNCTION clone_appinfoQ
* 
* Copy the contents of the src appinfoQ to the new appinfoQ
* Also add in any merge appinfo that are present
*
* INPUTS:
*    newQ == Q of ncx_appinfo_t getting new contents
*    srcQ == Q of ncx_appinfo_t with starting contents
*    merQ == Q of ncx_appinfo_t to merge into the clone,
*            as additions    (May be NULL)
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    clone_appinfoQ (dlq_hdr_t *newQ,
		    dlq_hdr_t *srcQ,
		    dlq_hdr_t *merQ)
{
    ncx_appinfo_t  *newapp, *srcapp;

    for (srcapp = (ncx_appinfo_t *)dlq_firstEntry(srcQ);
	 srcapp != NULL;
	 srcapp = (ncx_appinfo_t *)dlq_nextEntry(srcapp)) {

	newapp = ncx_clone_appinfo(srcapp);
	if (!newapp) {
	    log_error("\nError: clone of appinfo failed");
	    return ERR_INTERNAL_MEM;
	} else {
	    dlq_enque(newapp, newQ);
	}
    }

    if (merQ) {
	for (srcapp = (ncx_appinfo_t *)dlq_firstEntry(merQ);
	     srcapp != NULL;
	     srcapp = (ncx_appinfo_t *)dlq_nextEntry(srcapp)) {

	    newapp = ncx_clone_appinfo(srcapp);
	    if (!newapp) {
		log_error("\nError: clone of appinfo failed");
		return ERR_INTERNAL_MEM;
	    } else {
		dlq_enque(newapp, newQ);
	    }
	}
    }

    return NO_ERR;

}  /* clone_appinfoQ */


/********************************************************************
* FUNCTION clone_case
* 
* Clone a case struct
*
* INPUTS:
*    mod == module owner of the cloned data
*    cas == obj_case_t data structure to clone
*    mcas == obj_refine_t data structure to merge
*            into the clone, as refinements.  Only
*            legal case refinements will be checked
*            (May be NULL)
*    obj == new object template getting this cloned case
*    mobjQ == Q containing OBJ_TYP_REFINE objects to check
*
* RETURNS:
*   pointer to malloced case clone
*   NULL if  malloc error or internal error
*********************************************************************/
static obj_case_t *
    clone_case (ncx_module_t *mod,
		obj_case_t *cas,
		obj_refine_t *mcas,
		obj_template_t *obj,
		dlq_hdr_t  *mobjQ)
{
    obj_case_t     *newcas;
    status_t        res;

    res = NO_ERR;

    newcas = new_case(TRUE);  /*** need a real datadefQ ***/
    if (!newcas) {
	return NULL;
    }

    /* set the fields that cannot be refined */
    newcas->name = cas->name;
    newcas->nameclone = TRUE;
    newcas->status = cas->status;

    if (mcas && mcas->descr) {
	newcas->descr = xml_strdup(mcas->descr);
	if (!newcas->descr) {
	    free_case(newcas);
	    return NULL;
	}
    } else if (cas->descr) {
	newcas->descr = xml_strdup(cas->descr);
	if (!newcas->descr) {
	    free_case(newcas);
	    return NULL;
	}
    }

    if (mcas && mcas->ref) {
	newcas->ref = xml_strdup(mcas->ref);
	if (!newcas->ref) {
	    free_case(newcas);
	    return NULL;
	}
    } else if (cas->ref) {
	newcas->ref = xml_strdup(cas->ref);
	if (!newcas->ref) {
	    free_case(newcas);
	    return NULL;
	}
    }

    res = clone_datadefQ(mod, 
                         newcas->datadefQ, 
                         cas->datadefQ,
			 mobjQ, 
                         obj);
    if (res != NO_ERR) {
	free_case(newcas);
	return NULL;
    }

    return newcas;

}  /* clone_case */


/********************************************************************
* FUNCTION clone_mustQ
* 
* Clone a Q of must clauses (ncx_errinfo_t structs)
*
* INPUTS:
*    newQ == Q getting new structs
*    srcQ == Q with starting contents
*    mergeQ == Q of refinements (additional must statements)
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    clone_mustQ (dlq_hdr_t *newQ,
		 dlq_hdr_t *srcQ,
		 dlq_hdr_t *mergeQ)
{

    xpath_pcb_t *srcmust, *newmust;

    for (srcmust = (xpath_pcb_t *)dlq_firstEntry(srcQ);
	 srcmust != NULL;
	 srcmust = (xpath_pcb_t *)dlq_nextEntry(srcmust)) {

	newmust = xpath_clone_pcb(srcmust);
	if (!newmust) {
	    return ERR_INTERNAL_MEM;
	} else {
	    dlq_enque(newmust, newQ);
	}
    }

    if (mergeQ) {
	for (srcmust = (xpath_pcb_t *)dlq_firstEntry(mergeQ);
	     srcmust != NULL;
	     srcmust = (xpath_pcb_t *)dlq_nextEntry(srcmust)) {

	    newmust = xpath_clone_pcb(srcmust);
	    if (!newmust) {
		return ERR_INTERNAL_MEM;
	    } else {
		dlq_enque(newmust, newQ);
	    }
	}
    }

    return NO_ERR;

}  /* clone_mustQ */


/********************************************************************
* FUNCTION new_container
* 
* Malloc and initialize the fields in a an obj_container_t
*
* INPUTS:
*  isreal == TRUE if this is for a real object
*          == FALSE if this is a cloned object
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
static obj_container_t * 
    new_container (boolean isreal)
{
    obj_container_t  *con;

    con = m__getObj(obj_container_t);
    if (!con) {
	return NULL;
    }
    (void)memset(con, 0x0, sizeof(obj_container_t));

    con->datadefQ = dlq_createQue();
    if (!con->datadefQ) {
	m__free(con);
	return NULL;
    }

    if (isreal) {
	con->typedefQ = dlq_createQue();
	if (!con->typedefQ) {
	    dlq_destroyQue(con->datadefQ);
	    m__free(con);
	    return NULL;
	}

	con->groupingQ = dlq_createQue();
	if (!con->groupingQ) {
	    dlq_destroyQue(con->datadefQ);
	    dlq_destroyQue(con->typedefQ);
	    m__free(con);
	    return NULL;
	}

	con->status = NCX_STATUS_CURRENT;
    }

    dlq_createSQue(&con->mustQ);

    return con;

}  /* new_container */


/********************************************************************
* FUNCTION free_container
* 
* Scrub the memory in a obj_container_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    con == obj_container_t data structure to free
*    flags == flags field from object freeing this container
*********************************************************************/
static void 
    free_container (obj_container_t *con,
		    uint32 flags)
{
    boolean notclone;

    notclone = (flags & OBJ_FL_CLONE) ? FALSE : TRUE;

    if (con->name && notclone) {
	m__free(con->name);
    }
    if (con->descr) {
	m__free(con->descr);
    }
    if (con->ref) {
	m__free(con->ref);
    }
    if (con->presence) {
	m__free(con->presence);
    }

    if (notclone) {
	typ_clean_typeQ(con->typedefQ);
	dlq_destroyQue(con->typedefQ);
	grp_clean_groupingQ(con->groupingQ);
	dlq_destroyQue(con->groupingQ);
    }

    if (!con->datadefclone) {
	obj_clean_datadefQ(con->datadefQ);
	dlq_destroyQue(con->datadefQ);
    }

    clean_mustQ(&con->mustQ);

    m__free(con);

}  /* free_container */


/********************************************************************
* FUNCTION clone_container
* 
* Clone a container struct
*
* INPUTS:
*    mod == module owner of the cloned data
*    parent == new object containing 'con'
*    con == obj_container_t data structure to clone
*    mcon == obj_refine_t data structure to merge
*            into the clone, as refinements.  Only
*            legal container refinements will be checked
*            (May be NULL)
*    mobjQ == starting merge object Q, passed through
*
* RETURNS:
*   pointer to malloced object clone
*   NULL if  malloc error or internal error
*********************************************************************/
static obj_container_t *
    clone_container (ncx_module_t *mod,
		     obj_template_t  *parent,
		     obj_container_t *con,
		     obj_refine_t *mcon,
		     dlq_hdr_t  *mobjQ)
{
    obj_container_t *newcon;
    status_t         res;

    newcon = new_container(FALSE);
    if (!newcon) {
	return NULL;
    }

    /* set the fields that cannot be refined */
    newcon->name = con->name;

    newcon->typedefQ = con->typedefQ;
    newcon->groupingQ = con->groupingQ;
    newcon->status = con->status;


    if (mcon && mcon->descr) {
	newcon->descr = xml_strdup(mcon->descr);
	if (!newcon->descr) {
	    free_container(newcon, OBJ_FL_CLONE);
	    return NULL;
	}
    } else if (con->descr) {
	newcon->descr = xml_strdup(con->descr);
	if (!newcon->descr) {
	    free_container(newcon, OBJ_FL_CLONE);
	    return NULL;
	}
    }

    if (mcon && mcon->ref) {
	newcon->ref = xml_strdup(mcon->ref);
	if (!newcon->ref) {
	    free_container(newcon, OBJ_FL_CLONE);
	    return NULL;
	}
    } else if (con->ref) {
	newcon->ref = xml_strdup(con->ref);
	if (!newcon->ref) {
	    free_container(newcon, OBJ_FL_CLONE);
	    return NULL;
	}
    }

    if (mcon && mcon->presence) {
	newcon->presence = xml_strdup(mcon->presence);
	if (!newcon->presence) {
	    free_container(newcon, OBJ_FL_CLONE);
	    return NULL;
	}
    } else if (con->presence) {
	newcon->presence = xml_strdup(con->presence);
	if (!newcon->presence) {
	    free_container(newcon, OBJ_FL_CLONE);
	    return NULL;
	}
    }

    res = clone_mustQ(&newcon->mustQ, 
                      &con->mustQ,
		      (mcon) ? &mcon->mustQ : NULL);
    if (res != NO_ERR) {
	free_container(newcon, OBJ_FL_CLONE);
	return NULL;
    }

    res = clone_datadefQ(mod, 
                         newcon->datadefQ, 
			 con->datadefQ, 
                         mobjQ, 
                         parent);
    if (res != NO_ERR) {
	free_container(newcon, OBJ_FL_CLONE);
	return NULL;
    }

    return newcon;

}  /* clone_container */


/********************************************************************
* FUNCTION new_leaf
* 
* Malloc and initialize the fields in a an obj_leaf_t
*
* INPUTS:
*  isreal == TRUE if this is for a real object
*          == FALSE if this is a cloned object
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
static obj_leaf_t * 
    new_leaf (boolean isreal)
{
    obj_leaf_t  *leaf;

    leaf = m__getObj(obj_leaf_t);
    if (!leaf) {
	return NULL;
    }

    (void)memset(leaf, 0x0, sizeof(obj_leaf_t));

    if (isreal) {
	leaf->typdef = typ_new_typdef();
	if (!leaf->typdef) {
	    m__free(leaf);
	    return NULL;
	}
	leaf->status = NCX_STATUS_CURRENT;
    }

    dlq_createSQue(&leaf->mustQ);

    return leaf;

}  /* new_leaf */


/********************************************************************
* FUNCTION free_leaf
* 
* Scrub the memory in a obj_leaf_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    leaf == obj_leaf_t data structure to free
*    flags == flags field from object freeing this leaf
*********************************************************************/
static void 
    free_leaf (obj_leaf_t *leaf,
	       uint32 flags)
{
    boolean notclone;

    notclone = (flags & OBJ_FL_CLONE) ? FALSE : TRUE;

    if (leaf->name && notclone) {
	m__free(leaf->name);
    }
    if (leaf->units && notclone) {
	m__free(leaf->units);
    }
    if (leaf->defval) {
	m__free(leaf->defval);
    }
    if (leaf->descr) {
	m__free(leaf->descr);
    }
    if (leaf->ref) {
	m__free(leaf->ref);
    }

    if (leaf->typdef && (leaf->typdef->class != NCX_CL_BASE)
	&& notclone) {
	typ_free_typdef(leaf->typdef);
    }

    clean_mustQ(&leaf->mustQ);

    m__free(leaf);

}  /* free_leaf */


/********************************************************************
* FUNCTION clone_leaf
* 
* Clone a leaf struct
*
* INPUTS:
*    leaf == obj_leaf_t data structure to clone
*    mleaf == obj_refine_t data structure to merge
*            into the clone, as refinements.  Only
*            legal leaf refinements will be checked
*            (May be NULL)
*
* RETURNS:
*   pointer to malloced object clone
*   NULL if  malloc error or internal error
*********************************************************************/
static obj_leaf_t *
    clone_leaf (obj_leaf_t *leaf,
		obj_refine_t *mleaf)
{
    obj_leaf_t      *newleaf;
    status_t         res;

    newleaf = new_leaf(FALSE);
    if (!newleaf) {
	return NULL;
    }

    /* set the fields that cannot be refined */
    newleaf->name = leaf->name;
    newleaf->units = leaf->units;
    newleaf->typdef = leaf->typdef;
    newleaf->status = leaf->status;

    if (mleaf && mleaf->def) {
	newleaf->defval = xml_strdup(mleaf->def);
	if (!newleaf->defval) {
	    free_leaf(newleaf, OBJ_FL_CLONE);
	    return NULL;
	}
    } else if (leaf->defval) {
	newleaf->defval = xml_strdup(leaf->defval);
	if (!newleaf->defval) {
	    free_leaf(newleaf, OBJ_FL_CLONE);
	    return NULL;
	}
    }

    if (mleaf && mleaf->descr) {
	newleaf->descr = xml_strdup(mleaf->descr);
	if (!newleaf->descr) {
	    free_leaf(newleaf, OBJ_FL_CLONE);
	    return NULL;
	}
    } else if (leaf->descr) {
	newleaf->descr = xml_strdup(leaf->descr);
	if (!newleaf->descr) {
	    free_leaf(newleaf, OBJ_FL_CLONE);
	    return NULL;
	}
    }

    if (mleaf && mleaf->ref) {
	newleaf->ref = xml_strdup(mleaf->ref);
	if (!newleaf->ref) {
	    free_leaf(newleaf, OBJ_FL_CLONE);
	    return NULL;
	}
    } else if (leaf->ref) {
	newleaf->ref = xml_strdup(leaf->ref);
	if (!newleaf->ref) {
	    free_leaf(newleaf, OBJ_FL_CLONE);
	    return NULL;
	}
    }

    res = clone_mustQ(&newleaf->mustQ, 
                      &leaf->mustQ,
		      (mleaf) ? &mleaf->mustQ : NULL);
    if (res != NO_ERR) {
	free_leaf(newleaf, OBJ_FL_CLONE);
	return NULL;
    }

    return newleaf;

}  /* clone_leaf */


/********************************************************************
* FUNCTION new_leaflist
* 
* Malloc and initialize the fields in a an obj_leaflist_t
*
* INPUTS:
*  isreal == TRUE if this is for a real object
*          == FALSE if this is a cloned object
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
static obj_leaflist_t * 
    new_leaflist (boolean isreal)
{
    obj_leaflist_t  *leaflist;

    leaflist = m__getObj(obj_leaflist_t);
    if (!leaflist) {
	return NULL;
    }

    (void)memset(leaflist, 0x0, sizeof(obj_leaflist_t));

    if (isreal) {
	leaflist->typdef = typ_new_typdef();
	if (!leaflist->typdef) {
	    m__free(leaflist);
	    return NULL;
	}
	leaflist->status = NCX_STATUS_CURRENT;
	leaflist->ordersys = TRUE;
    }

    dlq_createSQue(&leaflist->mustQ);

    return leaflist;

}  /* new_leaflist */


/********************************************************************
* FUNCTION free_leaflist
* 
* Scrub the memory in a obj_leaflist_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    leaflist == obj_leaflist_t data structure to free
*    flags == flags field from object freeing this leaf-list
*********************************************************************/
static void 
    free_leaflist (obj_leaflist_t *leaflist,
		   uint32 flags)
{
    boolean notclone;

    notclone = (flags & OBJ_FL_CLONE) ? FALSE : TRUE;

    if (leaflist->name && notclone) {
	m__free(leaflist->name);
    }
    if (leaflist->units && notclone) {
	m__free(leaflist->units);
    }
    if (leaflist->descr) {
	m__free(leaflist->descr);
    }
    if (leaflist->ref) {
	m__free(leaflist->ref);
    }

    if (leaflist->typdef && notclone) {
	typ_free_typdef(leaflist->typdef);
    }

    clean_mustQ(&leaflist->mustQ);

    m__free(leaflist);

}  /* free_leaflist */


/********************************************************************
* FUNCTION clone_leaflist
* 
* Clone a leaf-list struct
*
* INPUTS:
*    leaflist == obj_leaflist_t data structure to clone
*    mleaflist == obj_refine_t data structure to merge
*            into the clone, as refinements.  Only
*            legal leaf-list refinements will be checked
*            (May be NULL)
*
* RETURNS:
*   pointer to malloced object clone
*   NULL if  malloc error or internal error
*********************************************************************/
static obj_leaflist_t *
    clone_leaflist (obj_leaflist_t *leaflist,
		    obj_refine_t *mleaflist)
{
    obj_leaflist_t      *newleaflist;
    status_t             res;

    newleaflist = new_leaflist(FALSE);
    if (!newleaflist) {
	return NULL;
    }

    /* set the fields that cannot be refined */
    newleaflist->name = leaflist->name;
    newleaflist->units = leaflist->units;
    newleaflist->typdef = leaflist->typdef;
    newleaflist->ordersys = leaflist->ordersys;
    newleaflist->status = leaflist->status;

    if (mleaflist && mleaflist->descr) {
	newleaflist->descr = xml_strdup(mleaflist->descr);
	if (!newleaflist->descr) {
	    free_leaflist(newleaflist, OBJ_FL_CLONE);
	    return NULL;
	}
    } else if (leaflist->descr) {
	newleaflist->descr = xml_strdup(leaflist->descr);
	if (!newleaflist->descr) {
	    free_leaflist(newleaflist, OBJ_FL_CLONE);
	    return NULL;
	}
    }

    if (mleaflist && mleaflist->ref) {
	newleaflist->ref = xml_strdup(mleaflist->ref);
	if (!newleaflist->ref) {
	    free_leaflist(newleaflist, OBJ_FL_CLONE);
	    return NULL;
	}
    } else if (leaflist->ref) {
	newleaflist->ref = xml_strdup(leaflist->ref);
	if (!newleaflist->ref) {
	    free_leaflist(newleaflist, OBJ_FL_CLONE);
	    return NULL;
	}
    }

    res = clone_mustQ(&newleaflist->mustQ, &leaflist->mustQ,
		     (mleaflist) ? &mleaflist->mustQ : NULL);
    if (res != NO_ERR) {
	free_leaflist(newleaflist, OBJ_FL_CLONE);
	return NULL;
    }

    if (mleaflist && mleaflist->minelems_tkerr.mod) {
	newleaflist->minelems = mleaflist->minelems;
	newleaflist->minset = TRUE;
    } else {
	newleaflist->minelems = leaflist->minelems;
	newleaflist->minset = leaflist->minset;
    }

    if (mleaflist && mleaflist->maxelems_tkerr.mod) {
	newleaflist->maxelems = mleaflist->maxelems;
	newleaflist->maxset = TRUE;
    } else {
	newleaflist->maxelems = leaflist->maxelems;
	newleaflist->maxset = leaflist->maxset;
    }

    return newleaflist;

}  /* clone_leaflist */


/********************************************************************
* FUNCTION free_list
* 
* Scrub the memory in a obj_list_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    list == obj_list_t data structure to free
*    flags == flags field from object freeing this list
*********************************************************************/
static void 
    free_list (obj_list_t *list,
	       uint32 flags)
{
    obj_key_t     *key;
    obj_unique_t  *uni;
    boolean       notclone;

    notclone = (flags & OBJ_FL_CLONE) ? FALSE : TRUE;

    if (list->name && notclone) {
	m__free(list->name);
    }
    if (list->keystr && notclone) {
	m__free(list->keystr);
    }
    if (list->descr) {
	m__free(list->descr);
    }
    if (list->ref) {
	m__free(list->ref);
    }

    while (!dlq_empty(&list->keyQ)) {
	key = (obj_key_t *)dlq_deque(&list->keyQ);
	obj_free_key(key);
    }

    while (!dlq_empty(&list->uniqueQ)) {
	uni = (obj_unique_t *)dlq_deque(&list->uniqueQ);
	obj_free_unique(uni);
    }

    if (notclone) {
	typ_clean_typeQ(list->typedefQ);
	dlq_destroyQue(list->typedefQ);

	grp_clean_groupingQ(list->groupingQ);
	dlq_destroyQue(list->groupingQ);
    }

    if (!list->datadefclone) {
	obj_clean_datadefQ(list->datadefQ);
	dlq_destroyQue(list->datadefQ);
    }

    clean_mustQ(&list->mustQ);

    m__free(list);

}  /* free_list */


/********************************************************************
* FUNCTION new_list
* 
* Malloc and initialize the fields in a an obj_list_t
*
* INPUTS:
*  isreal == TRUE if this is for a real object
*          == FALSE if this is a cloned object
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
static obj_list_t * 
    new_list (boolean isreal)
{
    obj_list_t  *list;

    list = m__getObj(obj_list_t);
    if (!list) {
	return NULL;
    }
    (void)memset(list, 0x0, sizeof(obj_list_t));


    dlq_createSQue(&list->keyQ);
    dlq_createSQue(&list->uniqueQ);

    list->status = NCX_STATUS_CURRENT;
    list->ordersys = TRUE;

    if (isreal) {
	list->typedefQ = dlq_createQue();
	if (!list->typedefQ) {
	    m__free(list);
	    return NULL;
	}

	list->groupingQ = dlq_createQue();
	if (!list->groupingQ) {
	    dlq_destroyQue(list->typedefQ);
	    m__free(list);
	    return NULL;
	}

    }

    dlq_createSQue(&list->mustQ);

    list->datadefQ = dlq_createQue();
    if (!list->datadefQ) {
	free_list(list, (isreal) ? 0U : OBJ_FL_CLONE);
	return NULL;
    }

    return list;

}  /* new_list */


/********************************************************************
* FUNCTION clone_list
* 
* Clone a leaf-list struct
*
* INPUTS:
*    mod == module owner of the cloned data
*    newparent == new obj containing 'list'
*    srclist == obj_template_t data structure to clone
*    mlist == obj_refine_t data structure to merge
*            into the clone, as refinements.  Only
*            legal list refinements will be checked
*            (May be NULL)
*    mobjQ == Q of objects with OBJ_TYP_REFINE to apply
*
* RETURNS:
*   pointer to malloced object clone
*   NULL if  malloc error or internal error
*********************************************************************/
static obj_list_t *
    clone_list (ncx_module_t *mod,
		obj_template_t *newparent,
		obj_template_t *srclist,
		obj_refine_t *mlist,
		dlq_hdr_t *mobjQ)
{
    obj_list_t      *list, *newlist;
    status_t         res;

    newlist = new_list(FALSE);
    if (!newlist) {
	return NULL;
    }

    list = srclist->def.list;

    /* set the fields that cannot be refined */
    newlist->name = list->name;
    newlist->keystr = list->keystr;
    newlist->typedefQ = list->typedefQ;
    newlist->groupingQ = list->groupingQ;
    newlist->ordersys = list->ordersys;
    newlist->status = list->status;

    if (mlist && mlist->descr) {
	newlist->descr = xml_strdup(mlist->descr);
	if (!newlist->descr) {
	    free_list(newlist, OBJ_FL_CLONE);
	    return NULL;
	}
    } else if (list->descr) {
	newlist->descr = xml_strdup(list->descr);
	if (!newlist->descr) {
	    free_list(newlist, OBJ_FL_CLONE);
	    return NULL;
	}
    }

    if (mlist && mlist->ref) {
	newlist->ref = xml_strdup(mlist->ref);
	if (!newlist->ref) {
	    free_list(newlist, OBJ_FL_CLONE);
	    return NULL;
	}
    } else if (list->ref) {
	newlist->ref = xml_strdup(list->ref);
	if (!newlist->ref) {
	    free_list(newlist, OBJ_FL_CLONE);
	    return NULL;
	}
    }

    res = clone_mustQ(&newlist->mustQ, 
                      &list->mustQ,
                      (mlist) ? &mlist->mustQ : NULL);
    if (res != NO_ERR) {
	free_list(newlist, OBJ_FL_CLONE);
	return NULL;
    }

    if (mlist && mlist->minelems_tkerr.mod) {
	newlist->minelems = mlist->minelems;
	newlist->minset = TRUE;
    } else {
	newlist->minelems = list->minelems;
	newlist->minset = list->minset;
    }

    if (mlist && mlist->maxelems_tkerr.mod) {
	newlist->maxelems = mlist->maxelems;
	newlist->maxset = TRUE;
    } else {
	newlist->maxelems = list->maxelems;
	newlist->maxset = list->maxset;
    }

    res = clone_datadefQ(mod, 
                         newlist->datadefQ, 
                         list->datadefQ, 
			 mobjQ, 
                         newparent);
    if (res != NO_ERR) {
	free_list(newlist, OBJ_FL_CLONE);
	return NULL;
    }

    /* newlist->keyQ is still empty
     * newlist->uniqueQ is still empty
     * these are filled in by the yang_obj_resolve_final function
     */
    return newlist;

}  /* clone_list */


/********************************************************************
* FUNCTION new_case
* 
* Malloc and initialize the fields in a an obj_case_t
*
* INPUTS:
*    isreal == TRUE if this is for a real object
*            == FALSE if this is a cloned object
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
static obj_case_t * 
    new_case (boolean isreal)
{
    obj_case_t  *cas;

    cas = m__getObj(obj_case_t);
    if (!cas) {
	return NULL;
    }
    (void)memset(cas, 0x0, sizeof(obj_case_t));

    cas->status = NCX_STATUS_CURRENT;

    if (isreal) {
	cas->datadefQ = dlq_createQue();
	if (!cas->datadefQ) {
	    m__free(cas);
	    return NULL;
	}
    }

    return cas;

}  /* new_case */


/********************************************************************
* FUNCTION free_case
* 
* Clean and free the fields in a an obj_case_t, then free 
* the case struct
*
* INPUTS:
*    cas == case struct to free
*********************************************************************/
static void
    free_case (obj_case_t *cas)
{
    if (cas->name && !cas->nameclone) {
	m__free(cas->name);
    }
    if (cas->descr) {
	m__free(cas->descr);
    }
    if (cas->ref) {
	m__free(cas->ref);
    }
    if (!cas->datadefclone) {
	obj_clean_datadefQ(cas->datadefQ);
	dlq_destroyQue(cas->datadefQ);
    }

    m__free(cas);

}  /* free_case */


/********************************************************************
* FUNCTION new_choice
* 
* Malloc and initialize the fields in a an obj_choice_t
*
* INPUTS:
*  isreal == TRUE if this is for a real object
*          == FALSE if this is a cloned object
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
static obj_choice_t * 
    new_choice (boolean isreal)
{
    obj_choice_t  *ch;

    ch = m__getObj(obj_choice_t);
    if (!ch) {
	return NULL;
    }
    (void)memset(ch, 0x0, sizeof(obj_choice_t));

    ch->caseQ = dlq_createQue();
    if (!ch->caseQ) {
	m__free(ch);
	return NULL;
    }

    if (isreal) {
	ch->status = NCX_STATUS_CURRENT;
    }

    return ch;

}  /* new_choice */


/********************************************************************
* FUNCTION free_choice
* 
* Scrub the memory in a obj_choice_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    choic == obj_choice_t data structure to free
*    flags == flags field from object freeing this choice
*********************************************************************/
static void 
    free_choice (obj_choice_t *choic,
		 uint32 flags)
{
    boolean notclone;

    notclone = (flags & OBJ_FL_CLONE) ? FALSE : TRUE;

    if (choic->name && notclone) {
	m__free(choic->name);
    }
    if (choic->defval) {
	m__free(choic->defval);
    }
    if (choic->descr) {
	m__free(choic->descr);
    }
    if (choic->ref) {
	m__free(choic->ref);
    }

    if (!choic->caseQclone) {
	obj_clean_datadefQ(choic->caseQ);
	dlq_destroyQue(choic->caseQ);
    }

    m__free(choic);

}  /* free_choice */


/********************************************************************
* FUNCTION clone_choice
* 
* Clone a choice struct
*
* INPUTS:
*    mod == module owner of the cloned data
*    choic == obj_choice_t data structure to clone
*    mchoic == obj_choice_t data structure to merge
*            into the clone, as refinements.  Only
*            legal choice refinements will be checked
*            (May be NULL)
*    obj == parent object containing 'choic'
*    mobjQ == Q with OBJ_TYP_REFINE nodes to check
*
* RETURNS:
*   pointer to malloced object clone
*   NULL if  malloc error or internal error
*********************************************************************/
static obj_choice_t *
    clone_choice (ncx_module_t *mod,
		  obj_choice_t *choic,
		  obj_refine_t *mchoic,
		  obj_template_t *obj,
		  dlq_hdr_t  *mobjQ)
{
    obj_choice_t    *newchoic;
    status_t         res;

    newchoic = new_choice(FALSE);
    if (!newchoic) {
	return NULL;
    }

    /* set the fields that cannot be refined */
    newchoic->name = choic->name;
    newchoic->status = choic->status;

    if (mchoic && mchoic->def) {
	newchoic->defval = xml_strdup(mchoic->def);
	if (!newchoic->defval) {
	    free_choice(newchoic, OBJ_FL_CLONE);
	    return NULL;
	}
    } else if (choic->defval) {
	newchoic->defval = xml_strdup(choic->defval);
	if (!newchoic->defval) {
	    free_choice(newchoic, OBJ_FL_CLONE);
	    return NULL;
	}
    }

    if (mchoic && mchoic->descr) {
	newchoic->descr = xml_strdup(mchoic->descr);
	if (!newchoic->descr) {
	    free_choice(newchoic, OBJ_FL_CLONE);
	    return NULL;
	}
    } else if (choic->descr) {
	newchoic->descr = xml_strdup(choic->descr);
	if (!newchoic->descr) {
	    free_choice(newchoic, OBJ_FL_CLONE);
	    return NULL;
	}
    }

    if (mchoic && mchoic->ref) {
	newchoic->ref = xml_strdup(mchoic->ref);
	if (!newchoic->ref) {
	    free_choice(newchoic, OBJ_FL_CLONE);
	    return NULL;
	}
    } else if (choic->ref) {
	newchoic->ref = xml_strdup(choic->ref);
	if (!newchoic->ref) {
	    free_choice(newchoic, OBJ_FL_CLONE);
	    return NULL;
	}
    }

    res = clone_datadefQ(mod, 
                         newchoic->caseQ,
			 choic->caseQ, 
                         mobjQ, 
                         obj);
    if (res != NO_ERR) {
	free_choice(newchoic, OBJ_FL_CLONE);
	return NULL;
    }

    return newchoic;

}  /* clone_choice */


/********************************************************************
* FUNCTION new_uses
* 
* Malloc and initialize the fields in a obj_uses_t
*
* INPUTS:
*  isreal == TRUE if this is for a real object
*          == FALSE if this is a cloned object
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
static obj_uses_t * 
    new_uses (boolean isreal)
{
    obj_uses_t  *us;

    us = m__getObj(obj_uses_t);
    if (!us) {
	return NULL;
    }
    (void)memset(us, 0x0, sizeof(obj_uses_t));

    if (isreal) {
	us->status = NCX_STATUS_CURRENT;   /* default */
    }

    us->datadefQ = dlq_createQue();
    if (!us->datadefQ) {
	m__free(us);
	return NULL;
    }

    return us;

}  /* new_uses */


/********************************************************************
* FUNCTION free_uses
* 
* Scrub the memory in a obj_uses_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    us == obj_uses_t data structure to free
*********************************************************************/
static void 
    free_uses (obj_uses_t *us)
{
    if (us->prefix) {
	m__free(us->prefix);
    }
    if (us->name) {
	m__free(us->name);
    }
    if (us->descr) {
	m__free(us->descr);
    }
    if (us->ref) {
	m__free(us->ref);
    }

    obj_clean_datadefQ(us->datadefQ);
    dlq_destroyQue(us->datadefQ);

    m__free(us);

}  /* free_uses */


/********************************************************************
* FUNCTION new_refine
* 
* Malloc and initialize the fields in a obj_refine_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
static obj_refine_t * 
    new_refine (void)
{
    obj_refine_t  *refi;

    refi = m__getObj(obj_refine_t);
    if (!refi) {
	return NULL;
    }
    (void)memset(refi, 0x0, sizeof(obj_refine_t));

    dlq_createSQue(&refi->mustQ);

    return refi;

}  /* new_refine */


/********************************************************************
* FUNCTION free_refine
* 
* Scrub the memory in a obj_refine_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    refi == obj_refine_t data structure to free
*********************************************************************/
static void 
    free_refine (obj_refine_t *refi)
{
    if (refi->target) {
	m__free(refi->target);
    }
    if (refi->descr) {
	m__free(refi->descr);
    }
    if (refi->ref) {
	m__free(refi->ref);
    }
    if (refi->presence) {
	m__free(refi->presence);
    }
    if (refi->def) {
	m__free(refi->def);
    }

    clean_mustQ(&refi->mustQ);

    m__free(refi);

}  /* free_refine */


/********************************************************************
* FUNCTION new_augment
* 
* Malloc and initialize the fields in a obj_augment_t
*
* INPUTS:
*  isreal == TRUE if this is for a real object
*          == FALSE if this is a cloned object
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
static obj_augment_t * 
    new_augment (boolean isreal)
{
    obj_augment_t  *aug;

    aug = m__getObj(obj_augment_t);
    if (!aug) {
	return NULL;
    }
    (void)memset(aug, 0x0, sizeof(obj_augment_t));

    dlq_createSQue(&aug->datadefQ);

    if (isreal) {
	aug->status = NCX_STATUS_CURRENT;
    }

    return aug;

}  /* new_augment */


/********************************************************************
* FUNCTION free_augment
* 
* Scrub the memory in a obj_augment_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    aug == obj_augment_t data structure to free
*********************************************************************/
static void 
    free_augment (obj_augment_t *aug)
{
    if (aug->target) {
	m__free(aug->target);
    }

    if (aug->descr) {
	m__free(aug->descr);
    }
    if (aug->ref) {
	m__free(aug->ref);
    }

    obj_clean_datadefQ(&aug->datadefQ);

    m__free(aug);

}  /* free_augment */


/********************************************************************
* FUNCTION new_rpc
* 
* Malloc and initialize the fields in a an obj_rpc_t
*
* RETURNS:
*    pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
static obj_rpc_t * 
    new_rpc (void)
{
    obj_rpc_t  *rpc;

    rpc = m__getObj(obj_rpc_t);
    if (!rpc) {
	return NULL;
    }
    (void)memset(rpc, 0x0, sizeof(obj_rpc_t));

    dlq_createSQue(&rpc->typedefQ);
    dlq_createSQue(&rpc->groupingQ);
    dlq_createSQue(&rpc->datadefQ);
    rpc->status = NCX_STATUS_CURRENT;

    /* by default: set supported to true for
     * agent simulation mode; there will not be any
     * callbacks to load, but RPC message
     * processing based on the template will be done
     */
    rpc->supported = TRUE;

    return rpc;

}  /* new_rpc */


/********************************************************************
* FUNCTION free_rpc
* 
* Clean and free the fields in a an obj_rpc_t, then free 
* the RPC struct
*
* INPUTS:
*    rpc == RPC struct to free
*********************************************************************/
static void
    free_rpc (obj_rpc_t *rpc)
{
    if (rpc->name) {
	m__free(rpc->name);
    }
    if (rpc->descr) {
	m__free(rpc->descr);
    }
    if (rpc->ref) {
	m__free(rpc->ref);
    }
    typ_clean_typeQ(&rpc->typedefQ);
    grp_clean_groupingQ(&rpc->groupingQ);
    obj_clean_datadefQ(&rpc->datadefQ);

    m__free(rpc);

}  /* free_rpc */


/********************************************************************
* FUNCTION new_rpcio
* 
* Malloc and initialize the fields in a an obj_rpcio_t
* Fields are setup within the new obj_template_t, based
* on the values in rpcobj
*
* RETURNS:
*    pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
static obj_rpcio_t * 
    new_rpcio (void)
{
    obj_rpcio_t  *rpcio;

    rpcio = m__getObj(obj_rpcio_t);
    if (!rpcio) {
	return NULL;
    }
    (void)memset(rpcio, 0x0, sizeof(obj_rpcio_t));

    dlq_createSQue(&rpcio->typedefQ);
    dlq_createSQue(&rpcio->groupingQ);
    dlq_createSQue(&rpcio->datadefQ);
    return rpcio;

}  /* new_rpcio */


/********************************************************************
* FUNCTION free_rpcio
* 
* Clean and free the fields in a an obj_rpcio_t, then free 
* the RPC IO struct
*
* INPUTS:
*    rpcio == RPC IO struct to free
*********************************************************************/
static void
    free_rpcio (obj_rpcio_t *rpcio)
{
    if (rpcio->name) {
	m__free(rpcio->name);
    }
    typ_clean_typeQ(&rpcio->typedefQ);
    grp_clean_groupingQ(&rpcio->groupingQ);
    obj_clean_datadefQ(&rpcio->datadefQ);
    m__free(rpcio);

}  /* free_rpcio */


/********************************************************************
* FUNCTION new_notif
* 
* Malloc and initialize the fields in a an obj_notif_t
*
* RETURNS:
*    pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
static obj_notif_t * 
    new_notif (void)
{
    obj_notif_t  *notif;

    notif = m__getObj(obj_notif_t);
    if (!notif) {
	return NULL;
    }
    (void)memset(notif, 0x0, sizeof(obj_notif_t));

    dlq_createSQue(&notif->typedefQ);
    dlq_createSQue(&notif->groupingQ);
    dlq_createSQue(&notif->datadefQ);
    notif->status = NCX_STATUS_CURRENT;
    return notif;

}  /* new_notif */


/********************************************************************
* FUNCTION free_notif
* 
* Clean and free the fields in a an obj_notif_t, then free 
* the notification struct
*
* INPUTS:
*    notif == notification struct to free
*********************************************************************/
static void
    free_notif (obj_notif_t *notif)
{
    if (notif->name) {
	m__free(notif->name);
    }
    if (notif->descr) {
	m__free(notif->descr);
    }
    if (notif->ref) {
	m__free(notif->ref);
    }
    typ_clean_typeQ(&notif->typedefQ);
    grp_clean_groupingQ(&notif->groupingQ);
    obj_clean_datadefQ(&notif->datadefQ);

    m__free(notif);

}  /* free_notif */


/********************************************************************
* FUNCTION find_template
* 
* Find an object with the specified name
*
* INPUTS:
*    que == Q of obj_template_t to search
*    modname == module name that defines the obj_template_t
*            == NULL and first match will be done, and the
*               module ignored (Name instead of QName)
*    objname == object name to find
*    lookdeep == TRUE to check objects inside choices/cases
*                and match these nodes before matching the 
*                choice or case
*             == FALSE to match choice/case names right away
*    match == TRUE if a strncmp test desired
*             FALSE if a normal strcmp (full match) test desired
*    matchcount == address of return parameter match count
*                  (may be NULL)
* OUTPUTS:
*   if non-NULL:
*    *matchcount == number of parameters that matched
*                   only the first match will be returned
*
* RETURNS:
*    pointer to obj_template_t or NULL if not found in 'que'
*********************************************************************/
static obj_template_t *
    find_template (dlq_hdr_t  *que,
		   const xmlChar *modname,
		   const xmlChar *objname,
		   boolean lookdeep,
		   boolean match,
		   uint32 *matchcount)
{
    obj_template_t *obj, *chobj, *casobj, *matchobj;
    obj_case_t     *cas;
    const xmlChar  *name, *mname;
    uint32          len;
    int             ret;

    matchobj = NULL;
    len = 0;

    if (match) {
	len = xml_strlen(objname);
    }

    /* check all the objects in this datadefQ */
    for (obj = (obj_template_t *)dlq_firstEntry(que);
	 obj != NULL;
	 obj = (obj_template_t *)dlq_nextEntry(obj)) {

	/* skip augment and uses */
	if (!obj_has_name(obj) || !obj_is_enabled(obj)) {
	    continue;
	}

	name = obj_get_name(obj);
	mname = obj_get_mod_name(obj);

	if (match) {
	    ret = xml_strncmp(objname, name, len);
	} else {
	    ret = xml_strcmp(objname, name);
	}

	if (!lookdeep) {
	    /* if lookdeep == TRUE then look past
	     * OBJ_TYP_CHOICE and OBJ_TYP_CASE objects
	     * to see if any 'real' nodes match first
	     * If not, then the case, then choice
	     * name will be checked
	     */
	    if (modname && xml_strcmp(modname, mname)) {
		continue;
	    }
	    if (ret == 0) {
		if (match) {
                    if (obj->objtype == OBJ_TYP_CHOICE ||
                        obj->objtype == OBJ_TYP_CASE) {
                        ;
                    } else {
                        if (matchcount) {
                            (*matchcount)++;
                        }
                        matchobj = obj;
                    }
                } else {
		    return obj;
		}
	    }
	}

	switch (obj->objtype) {
	case OBJ_TYP_CHOICE:
	    /* since the choice and case layers disappear, need
	     * to check if any real node names would clash
	     * will also check later that all choice nodes
	     * within the same sibling set do not clash either
	     */
	    for (casobj = (obj_template_t *)
		     dlq_firstEntry(obj->def.choic->caseQ);
		 casobj != NULL;
		 casobj = (obj_template_t *)dlq_nextEntry(casobj)) {
		cas = casobj->def.cas;
		chobj = find_template(cas->datadefQ,
				      modname, 
                                      objname,
				      lookdeep, 
                                      match, 
                                      matchcount);
		if (chobj) {
		    if (match) {
			matchobj = chobj;
		    } else {
			return chobj;
		    }
		}
	    }

	    /* last try: the choice name itself */
	    if (ret == 0) {
		if (!match) {
		    return obj;
		}
	    }
	    break;
	case OBJ_TYP_CASE:
	    cas = obj->def.cas;
	    chobj = find_template(cas->datadefQ,
				  modname, 
                                  objname,
				  lookdeep, 
                                  match, 
                                  matchcount);
	    if (chobj) {
		if (match) {
		    matchobj = chobj;
		} else {
		    return chobj;
		}
	    } else if (ret == 0) {
		/* try case name itself */
		if (!match) {
		    return obj;
		}
	    }
	    break;
	default:
	    /* check if a specific module name requested,
	     * and if so, skip any object not from that module
	     */
	    if (lookdeep) {
		if (modname && xml_strcmp(modname, mname)) {
		    continue;
		}
		if (ret == 0) {
		    if (match) {
			if (matchcount) {
			    (*matchcount)++;
			}
			matchobj = obj;
		    } else {
			return obj;
		    }
		}
	    }
	}
    }

    if (match) {
	return matchobj;
    } else {
	return NULL;
    }

}  /* find_template */


/********************************************************************
* FUNCTION get_config_flag
*
* Get the config flag for an obj_template_t 
* Return the explicit value or the inherited value
* Also return if the config-stmt is really set or not
*
* INPUTS:
*   obj == obj_template to check
*   setflag == address of return config-stmt set flag
*
* OUTPUTS:
*   *setflag == TRUE if the config-stmt is set in this
*               node, or if it is a top-level object
*            == FALSE if the config-stmt is inherited from its parent
*
* RETURNS:
*   TRUE if config set to TRUE
*   FALSE if config set to FALSE
*   
*********************************************************************/
static boolean
    get_config_flag (const obj_template_t *obj,
		     boolean *setflag)
{
    switch (obj->objtype) {
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_CONTAINER:
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
    case OBJ_TYP_LIST:
    case OBJ_TYP_CHOICE:
	if (obj_is_root(obj)) {
	    *setflag = TRUE;
	    return TRUE;
	} else if ((obj->parent && 
		    !obj_is_root(obj->parent)) || obj->grp) {
	    *setflag = (obj->flags & OBJ_FL_CONFSET) 
		? TRUE : FALSE;
	} else {
	    *setflag = TRUE;
	}
	return (obj->flags & OBJ_FL_CONFIG) ? TRUE : FALSE;
    case OBJ_TYP_CASE:
	*setflag = FALSE;
	if (obj->parent) {
	    return (obj->parent->flags & OBJ_FL_CONFIG)
		? TRUE : FALSE;
	} else {
	    /* should not happen */
	    return FALSE;
	}
	/*NOTREACHED*/
    case OBJ_TYP_USES:
    case OBJ_TYP_AUGMENT:
    case OBJ_TYP_REFINE:
	/* no real setting -- not applicable */
	*setflag = FALSE;
	return FALSE;
    case OBJ_TYP_RPC:
	/* no real setting for this, but has to be true
	 * to allow rpc/input to be true
	 */
	*setflag = FALSE;
	return TRUE;
    case OBJ_TYP_RPCIO:
	*setflag = FALSE;
	if (!xml_strcmp(obj->def.rpcio->name, YANG_K_INPUT)) {
	    return TRUE;
	} else {
	    return FALSE;
	}
	break;
    case OBJ_TYP_NOTIF:
	*setflag = FALSE;
	return FALSE;
    case OBJ_TYP_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

    /*NOTREACHED*/

}   /* get_config_flag */


/********************************************************************
* FUNCTION get_object_string
* 
* Generate the object identifier string
* 
* INPUTS:
*   obj == node to generate the instance ID for
*   buff == buffer to use (may be NULL)
*   bufflen == length of buffer to use (0 to ignore check)
*   normalmode == TRUE for a real Xpath OID
*              == FALSE to generate a big element name to use
*                 for augment substitutionGroup name generation
*   mod == module in progress for C code generation
*       == NULL for other purposes
*   retlen == address of return length
*
* OUTPUTS
*   *retlen == length of object identifier string
*   if buff non-NULL:
*      *buff filled in with string
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    get_object_string (const obj_template_t *obj,
		       xmlChar  *buff,
		       uint32 bufflen,
		       boolean normalmode,
                       ncx_module_t *mod,
		       uint32 *retlen)
{
    const xmlChar *name, *modname;
    uint32         namelen, modnamelen;
    status_t       res;
    boolean        addmodname;

    *retlen = 0;

    addmodname = FALSE;

    if (obj->parent && !obj_is_root(obj->parent)) {
	res = get_object_string(obj->parent, 
                                buff, 
				bufflen, 
                                normalmode,
                                mod,
                                retlen);
	if (res != NO_ERR) {
	    return res;
	}
    }

    if (!obj_has_name(obj)) {
	/* should not enounter a uses or augment!! */
	return NO_ERR;
    }

    modname = obj_get_mod_name(obj);
    modnamelen = xml_strlen(modname);

    if (mod != NULL &&
        (xml_strcmp(modname, ncx_get_modname(mod)))) {
        addmodname = TRUE;
    }

    /* get the name and check the added length */
    name = obj_get_name(obj);
    namelen = xml_strlen(name);

    if (addmodname) {
        if (bufflen &&
            ((*retlen + namelen + modnamelen + 2) >= bufflen)) {
            return ERR_BUFF_OVFL;
        }
    } else {
        if (bufflen && ((*retlen + namelen + 1) >= bufflen)) {
            return ERR_BUFF_OVFL;
        }
    }

    /* copy the name string recusively, letting the stack
     * keep track of the next child node to write 
     */
    if (buff) {
	/* node separator char */
	if (normalmode) {
	    buff[*retlen] = '/';
	} else {
	    buff[*retlen] = '.';
	}
        if (addmodname) {
            xml_strcpy(&buff[*retlen + 1], modname);
            buff[*retlen + modnamelen + 2] = '_';
            xml_strcpy(&buff[*retlen + modnamelen + 3], name);
        } else {
            xml_strcpy(&buff[*retlen + 1], name);
        }
    }
    if (addmodname) {
        *retlen += (namelen + modnamelen + 2);
    } else {
        *retlen += (namelen + 1);
    }
    return NO_ERR;

}  /* get_object_string */


/********************************************************************
 * FUNCTION find_next_child
 * 
 * Check the instance qualifiers and see if the specified node
 * is a valid (subsequent) child node.
 *
 * Example:
 *  
 *  container foo { 
 *    leaf a { type int32; }
 *    leaf b { type int32; }
 *    leaf-list c { type int32; }
 *
 * Since a, b, and c are all optional, all of them have to be
 * checked, even while node 'a' is expected
 * The caller will save the current child in case the pointer
 * needs to be backed up.
 *
 * INPUTS:
 *   chobj == current child object template
 *   chnode == xml_node_t of start element to match
 *
 * RETURNS:
 *   pointer to child that matched or NULL if no valid next child
 *********************************************************************/
static obj_template_t *
    find_next_child (obj_template_t *chobj,
		     const xml_node_t *chnode)
{

    obj_template_t *chnext, *foundobj;
    status_t        res;

    chnext = chobj;

    for (;;) {
	switch (obj_get_iqualval(chnext)) {
	case NCX_IQUAL_ONE:
	case NCX_IQUAL_1MORE:
	    /* the current child is mandatory; this is an error */
	    return NULL;
	    /* else fall through to next case */
	case NCX_IQUAL_OPT:
	case NCX_IQUAL_ZMORE:
	    /* the current child is optional; keep trying
	     * try to get the next child in the complex type 
	     */
	    chnext = obj_next_child(chnext);
	    if (!chnext) {
		return NULL;
	    } else {
		if (chnext->objtype==OBJ_TYP_CHOICE ||
		    chnext->objtype==OBJ_TYP_CASE) {
		    foundobj = obj_find_child(chnext,
					      xmlns_get_module(chnode->nsid),
					      chnode->elname);
		    if (foundobj && 
			(foundobj->objtype==OBJ_TYP_CHOICE ||
			 foundobj->objtype==OBJ_TYP_CASE)) {
			foundobj = NULL;
		    }
		    if (foundobj) {
			return chnext;  /* not the nested foundobj! */
		    }
		} else {
		    res = xml_node_match(chnode,
					 obj_get_nsid(chnext), 
					 obj_get_name(chnext), 
					 XML_NT_NONE);
		    if (res == NO_ERR) {
			return chnext;
		    }
		}
	    }
	    break;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	    return NULL;
	}
    }
    /*NOTREACHED*/

} /* find_next_child */

/********************************************************************
* FUNCTION free_blank_template
* 
* Free a blank obj_template_t
*
* INPUTS:
*   obj == blank template to free
*
*********************************************************************/
static void
    free_blank_template (obj_template_t *obj)
{

    clean_metadataQ(&obj->metadataQ);
    ncx_clean_appinfoQ(&obj->appinfoQ);
    m__free(obj);

}  /* free_blank_template */


/********************************************************************
* FUNCTION process_one_walker_child
* 
* Process one child object node 
*
* INPUTS:
*    walkerfn == callback function to use
*    cookie1 == cookie1 value to pass to walker fn
*    cookie2 == cookie2 value to pass to walker fn
*    obj == object to process
*    modname == module name; 
*                the first match in this module namespace
*                will be returned
*            == NULL:
*                 the first match in any namespace will
*                 be  returned;
*    childname == name of child node to find
*              == NULL to match any child name
*    configonly = TRUE for config=true only
*    textmode == TRUE if just testing for text() nodes
*                name and modname will be ignored in this mode
*                FALSE if using name and modname to filter
*    fncalled == address of return function called flag
*
* RETURNS:
*   TRUE if normal termination occurred
*   FALSE if walker fn requested early termination
*********************************************************************/
static boolean
    process_one_walker_child (obj_walker_fn_t walkerfn,
			      void *cookie1,
			      void *cookie2,
			      obj_template_t  *obj,
			      const xmlChar *modname,
			      const xmlChar *childname,
			      boolean configonly,
			      boolean textmode,
			      boolean *fncalled)
			      
{
    boolean         fnresult;

    *fncalled = FALSE;
    if (!obj_has_name(obj)) {
	return TRUE;
    }

    if (configonly && !childname && 
	!obj_is_config(obj)) {
	return TRUE;
    }

    fnresult = TRUE;
    if (textmode) {
	if (obj_is_leafy(obj)) {
	    fnresult = (*walkerfn)(obj, cookie1, cookie2);
	    *fncalled = TRUE;
	}
    } else if (modname && childname) {
	if (!xml_strcmp(modname, 
			obj_get_mod_name(obj)) &&
	    !xml_strcmp(childname, obj_get_name(obj))) {

	    fnresult = (*walkerfn)(obj, cookie1, cookie2);
	    *fncalled = TRUE;
	}
    } else if (modname) {
	if (!xml_strcmp(modname, obj_get_mod_name(obj))) {
	    fnresult = (*walkerfn)(obj, cookie1, cookie2);
	    *fncalled = TRUE;
	}
    } else if (childname) {
	if (!xml_strcmp(childname, obj_get_name(obj))) {
	    fnresult = (*walkerfn)(obj, cookie1, cookie2);
	    *fncalled = TRUE;
	}
    } else {
	fnresult = (*walkerfn)(obj, cookie1, cookie2);
	*fncalled = TRUE;
    }

    return fnresult;

}  /* process_one_walker_child */


/********************************************************************
* FUNCTION test_one_child
* 
* The the specified node
* The walker fn will be called for each match.  
*
* If the walker function returns TRUE, then the 
* walk will continue; If FALSE it will terminate right away
*
* This function skips choice and case nodes and
* only processes real data nodes
*
* INPUTS:
*    exprmod == module containing the XPath object
*    walkerfn == callback function to use
*    cookie1 == cookie1 value to pass to walker fn
*    cookie2 == cookie2 value to pass to walker fn
*    obj == node to check
*    modname == module name; 
*                only matches in this module namespace
*                will be returned
*            == NULL:
*                 namespace matching will be skipped
*    name == name of node to find
*              == NULL to match any name
*    configonly == TRUE to skip over non-config nodes
*                  FALSE to check all nodes
*                  Only used if childname == NULL
*    textmode == TRUE if just testing for text() nodes
*                name and modname will be ignored in this mode
*                FALSE if using name and modname to filter
*
* RETURNS:
*   TRUE if normal termination occurred
*   FALSE if walker fn requested early termination
*********************************************************************/
static boolean
    test_one_child (ncx_module_t *exprmod,
		    obj_walker_fn_t walkerfn,
		    void *cookie1,
		    void *cookie2,
		    obj_template_t *obj,
		    const xmlChar *modname,
		    const xmlChar *name,
		    boolean configonly,
		    boolean textmode)
{
    boolean               fnresult, fncalled;

    if (obj->objtype == OBJ_TYP_CHOICE ||
	obj->objtype == OBJ_TYP_CASE) {
	fnresult = obj_find_all_children(exprmod,
					 walkerfn,
					 cookie1,
					 cookie2,
					 obj,
					 modname,
					 name,
					 configonly,
					 textmode,
					 FALSE);
    } else {
	fnresult = process_one_walker_child(walkerfn,
					    cookie1,
					    cookie2,
					    obj,
					    modname, 
					    name,
					    configonly,
					    textmode,
					    &fncalled);
    }

    if (!fnresult) {
	return FALSE;
    }

    return TRUE;

}  /* test_one_child */


/********************************************************************
* FUNCTION test_one_ancestor
* 
* The the specified node
* The walker fn will be called for each match.  
*
* If the walker function returns TRUE, then the 
* walk will continue; If FALSE it will terminate right away
*
* This function skips choice and case nodes and
* only processes real data nodes
*
* INPUTS:
*    exprmod == module containing object
*    walkerfn == callback function to use
*    cookie1 == cookie1 value to pass to walker fn
*    cookie2 == cookie2 value to pass to walker fn
*    obj == node to check
*    modname == module name; 
*                only matches in this module namespace
*                will be returned
*            == NULL:
*                 namespace matching will be skipped
*    name == name of node to find
*              == NULL to match any name
*    configonly == TRUE to skip over non-config nodes
*                  FALSE to check all nodes
*                  Only used if childname == NULL
*    textmode == TRUE if just testing for text() nodes
*                name and modname will be ignored in this mode
*                FALSE if using name and modname to filter
*    orself == TRUE if axis ancestor-or-self
*    fncalled == address of return function called flag
*
* OUTPUTS:
*   *fncalled set to TRUE if a callback function was called
*
* RETURNS:
*   TRUE if normal termination occurred
*   FALSE if walker fn requested early termination
*********************************************************************/
static boolean
    test_one_ancestor (ncx_module_t *exprmod,
		       obj_walker_fn_t walkerfn,
		       void *cookie1,
		       void *cookie2,
		       obj_template_t *obj,
		       const xmlChar *modname,
		       const xmlChar *name,
		       boolean configonly,
		       boolean textmode,
		       boolean orself,
		       boolean *fncalled)
{
    boolean               fnresult;

    if (obj->objtype == OBJ_TYP_CHOICE ||
	obj->objtype == OBJ_TYP_CASE) {
	fnresult = obj_find_all_ancestors(exprmod,
					  walkerfn,
					  cookie1,
					  cookie2,
					  obj,
					  modname,
					  name,
					  configonly,
					  textmode,
					  FALSE,
					  orself,
					  fncalled);
    } else {
	fnresult = process_one_walker_child(walkerfn,
					    cookie1,
					    cookie2,
					    obj,
					    modname, 
					    name,
					    configonly,
					    textmode,
					    fncalled);
    }

    if (!fnresult) {
	return FALSE;
    }

    return TRUE;

}  /* test_one_ancestor */


/********************************************************************
* FUNCTION test_one_descendant
* 
* The the specified node
* The walker fn will be called for each match.  
*
* If the walker function returns TRUE, then the 
* walk will continue; If FALSE it will terminate right away
*
* This function skips choice and case nodes and
* only processes real data nodes
*
* INPUTS:
*    exprmod == module containing object
*    walkerfn == callback function to use
*    cookie1 == cookie1 value to pass to walker fn
*    cookie2 == cookie2 value to pass to walker fn
*    startobj == node to check
*    modname == module name; 
*                only matches in this module namespace
*                will be returned
*            == NULL:
*                 namespace matching will be skipped
*    name == name of node to find
*              == NULL to match any name
*    configonly == TRUE to skip over non-config nodes
*                  FALSE to check all nodes
*                  Only used if childname == NULL
*    textmode == TRUE if just testing for text() nodes
*                name and modname will be ignored in this mode
*                FALSE if using name and modname to filter
*    orself == TRUE if descendant-or-self test
*    fncalled == address of return function called flag
*
* OUTPUTS:
*   *fncalled set to TRUE if a callback function was called

* RETURNS:
*   TRUE if normal termination occurred
*   FALSE if walker fn requested early termination
*********************************************************************/
static boolean
    test_one_descendant (ncx_module_t *exprmod,
			 obj_walker_fn_t walkerfn,
			 void *cookie1,
			 void *cookie2,
			 obj_template_t *startobj,
			 const xmlChar *modname,
			 const xmlChar *name,
			 boolean configonly,
			 boolean textmode,
			 boolean orself,
			 boolean *fncalled)
{
    obj_template_t *obj;
    dlq_hdr_t      *datadefQ;
    boolean         fnresult;

    if (orself) {
	fnresult = process_one_walker_child(walkerfn,
					    cookie1,
					    cookie2,
					    startobj,
					    modname, 
					    name,
					    configonly,
					    textmode,
					    fncalled);
	if (!fnresult) {
	    return FALSE;
	}
    }

    datadefQ = obj_get_datadefQ(startobj);
    if (!datadefQ) {
	return TRUE;
    }

    for (obj = (obj_template_t *)dlq_firstEntry(datadefQ);
	 obj != NULL;
	 obj = (obj_template_t *)dlq_nextEntry(obj)) {

	if (obj->objtype == OBJ_TYP_CHOICE ||
	    obj->objtype == OBJ_TYP_CASE) {
	    fnresult = obj_find_all_descendants(exprmod,
						walkerfn,
						cookie1,
						cookie2,
						obj,
						modname,
						name,
						configonly,
						textmode,
						FALSE,
						orself,
						fncalled);
	} else {
	    fnresult = process_one_walker_child(walkerfn,
						cookie1,
						cookie2,
						obj,
						modname, 
						name,
						configonly,
						textmode,
						fncalled);
	    if (fnresult && !*fncalled) {
		fnresult = obj_find_all_descendants(exprmod,
						    walkerfn,
						    cookie1,
						    cookie2,
						    obj,
						    modname,
						    name,
						    configonly,
						    textmode,
						    FALSE,
						    orself,
						    fncalled);
	    }
	}
	if (!fnresult) {
	    return FALSE;
	}
    }

    return TRUE;

}  /* test_one_descendant */


/********************************************************************
* FUNCTION test_one_pfnode
* 
* The the specified node
* The walker fn will be called for each match.  
*
* If the walker function returns TRUE, then the 
* walk will continue; If FALSE it will terminate right away
*
* This function skips choice and case nodes and
* only processes real data nodes
*
* INPUTS:
*    exprmod == module containing object
*    walkerfn == callback function to use
*    cookie1 == cookie1 value to pass to walker fn
*    cookie2 == cookie2 value to pass to walker fn
*    obj == node to check
*    modname == module name; 
*                only matches in this module namespace
*                will be returned
*            == NULL:
*                 namespace matching will be skipped
*    childname == name of child node to find
*              == NULL to match any child name
*    configonly == TRUE to skip over non-config nodes
*                  FALSE to check all nodes
*                  Only used if childname == NULL
*    dblslash == TRUE if all decendents of the preceding
*                 or following nodes should be checked
*                FALSE only 1 level is checked
*    textmode == TRUE if just testing for text() nodes
*                name and modname will be ignored in this mode
*                FALSE if using name and modname to filter
*    forward == TRUE: forward axis test
*               FALSE: reverse axis test
*    axis == axis enum to use
*    fncalled == address of return function called flag
*
* OUTPUTS:
*   *fncalled set to TRUE if a callback function was called
*
* RETURNS:
*   TRUE if normal termination occurred
*   FALSE if walker fn requested early termination
*********************************************************************/
static boolean
    test_one_pfnode (ncx_module_t *exprmod,
		     obj_walker_fn_t walkerfn,
		     void *cookie1,
		     void *cookie2,
		     obj_template_t *obj,
		     const xmlChar *modname,
		     const xmlChar *name,
		     boolean configonly,
		     boolean dblslash,
		     boolean textmode,
		     boolean forward,
		     ncx_xpath_axis_t axis,
		     boolean *fncalled)
{
    obj_template_t *child;
    boolean         fnresult, needcont;

    /* for objects, need to let same node match, because
     * if there are multiple instances of the object,
     * it would match
     */
    if (obj->objtype == OBJ_TYP_LIST ||
	obj->objtype == OBJ_TYP_LEAF_LIST) {
	;
    } else if (forward) {
	obj = (obj_template_t *)dlq_nextEntry(obj);
    } else {
	obj = (obj_template_t *)dlq_prevEntry(obj);
    }

    while (obj) {
	needcont = FALSE;

	if (!obj_has_name(obj)) {
	    needcont = TRUE;
	}

	if (configonly && !name && !obj_is_config(obj)) {
	    needcont = TRUE;
	}

	if (needcont) {
	    /* get the next node to process */
	    if (forward) {
		obj = (obj_template_t *)dlq_nextEntry(obj);
	    } else {
		obj = (obj_template_t *)dlq_prevEntry(obj);
	    }
	    continue;
	}

	if (obj->objtype == OBJ_TYP_CHOICE ||
	    obj->objtype == OBJ_TYP_CASE) {
	    for (child = (forward) ? obj_first_child(obj) :
		     obj_last_child(obj);
		 child != NULL;
		 child = (forward) ? obj_next_child(child) :
		     obj_previous_child(child)) {

		fnresult = obj_find_all_pfaxis(exprmod,
					       walkerfn,
					       cookie1,
					       cookie2,
					       child,
					       modname,
					       name,
					       configonly,
					       dblslash,
					       textmode,
					       FALSE,
					       axis,
					       fncalled);
		if (!fnresult) {
		    return FALSE;
		}
	    }
	} else {
	    fnresult = process_one_walker_child(walkerfn,
						cookie1,
						cookie2,
						obj,
						modname,
						name,
						configonly,
						textmode,
						fncalled);
	    if (!fnresult) {
		return FALSE;
	    }

	    if (!*fncalled && dblslash) {
		/* if /foo did not get added, than 
		 * try /foo/bar, /foo/baz, etc.
		 * check all the child nodes even if
		 * one of them matches, because all
		 * matches are needed with the '//' operator
		 */
		for (child = (forward) ?
			 obj_first_child(obj) :
			 obj_last_child(obj);
		     child != NULL;
		     child = (forward) ?
			 obj_next_child(child) :
			 obj_previous_child(child)) {

		    fnresult = 
			obj_find_all_pfaxis(exprmod,
					    walkerfn, 
					    cookie1, 
					    cookie2,
					    child, 
					    modname, 
					    name, 
					    configonly,
					    dblslash,
					    textmode,
					    FALSE,
					    axis,
					    fncalled);
		    if (!fnresult) {
			return FALSE;
		    }
		}
	    }
	}

	/* get the next node to process */
	if (forward) {
	    obj = (obj_template_t *)dlq_nextEntry(obj);
	} else {
	    obj = (obj_template_t *)dlq_prevEntry(obj);
	}

    }
    return TRUE;

}  /* test_one_pfnode */


/**************** E X T E R N A L    F U N C T I O N S    **********/


/********************************************************************
* FUNCTION obj_new_template
* 
* Malloc and initialize the fields in a an object template
*
* INPUTS:
*   objtype == the specific object type to create
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
obj_template_t * 
    obj_new_template (obj_type_t objtype)
{
    obj_template_t  *obj;

#ifdef DEBUG
    if (!objtype || objtype > OBJ_TYP_NOTIF) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
#endif

    obj = m__getObj(obj_template_t);
    if (!obj) {
	return NULL;
    }

    (void)memset(obj, 0x0, sizeof(obj_template_t));
    obj->objtype = objtype;
    dlq_createSQue(&obj->metadataQ);
    dlq_createSQue(&obj->appinfoQ);
    dlq_createSQue(&obj->iffeatureQ);
    
    switch (objtype) {
    case OBJ_TYP_CONTAINER:
	obj->def.container = new_container(TRUE);
	if (!obj->def.container) {
	    m__free(obj);
	    return NULL;
	}
	break;
    case OBJ_TYP_LEAF:
    case OBJ_TYP_ANYXML:
	obj->def.leaf = new_leaf(TRUE);
	if (!obj->def.leaf) {
	    m__free(obj);
	    return NULL;
	}
	break;
    case OBJ_TYP_LEAF_LIST:
	obj->def.leaflist = new_leaflist(TRUE);
	if (!obj->def.leaflist) {
	    m__free(obj);
	    return NULL;
	}
	break;
    case OBJ_TYP_LIST:
	obj->def.list = new_list(TRUE);
	if (!obj->def.list) {
	    m__free(obj);
	    return NULL;
	}
	break;
    case OBJ_TYP_CHOICE:
	obj->def.choic = new_choice(TRUE);
	if (!obj->def.choic) {
	    m__free(obj);
	    return NULL;
	}
	break;
    case OBJ_TYP_CASE:
	obj->def.cas = new_case(TRUE);
	if (!obj->def.cas) {
	    m__free(obj);
	    return NULL;
	}
	break;
    case OBJ_TYP_USES:
	obj->def.uses = new_uses(TRUE);
	if (!obj->def.uses) {
	    m__free(obj);
	    return NULL;
	}
	break;
    case OBJ_TYP_REFINE:
	obj->def.refine = new_refine();
	if (!obj->def.refine) {
	    m__free(obj);
	    return NULL;
	}
	break;
    case OBJ_TYP_AUGMENT:
	obj->def.augment = new_augment(TRUE);
	if (!obj->def.augment) {
	    m__free(obj);
	    return NULL;
	}
	break;
    case OBJ_TYP_RPC:
	obj->def.rpc = new_rpc();
	if (!obj->def.rpc) {
	    m__free(obj);
	    return NULL;
	}
	break;
    case OBJ_TYP_RPCIO:
	obj->def.rpcio = new_rpcio();
	if (!obj->def.rpcio) {
	    m__free(obj);
	    return NULL;
	}
	break;
    case OBJ_TYP_NOTIF:
	obj->def.notif = new_notif();
	if (!obj->def.notif) {
	    m__free(obj);
	    return NULL;
	}
	break;
    case OBJ_TYP_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

    return obj;

}  /* obj_new_template */


/********************************************************************
* FUNCTION obj_free_template
* 
* Scrub the memory in a obj_template_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    obj == obj_template_t data structure to free
*********************************************************************/
void 
    obj_free_template (obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    clean_metadataQ(&obj->metadataQ);
    ncx_clean_appinfoQ(&obj->appinfoQ);
    ncx_clean_iffeatureQ(&obj->iffeatureQ);

    if (obj->when) {
	xpath_free_pcb(obj->when);
    }

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	if (obj->def.container) {
	    free_container(obj->def.container, obj->flags);
	}
	break;
    case OBJ_TYP_LEAF:
    case OBJ_TYP_ANYXML:
	if (obj->def.leaf) {
	    free_leaf(obj->def.leaf, obj->flags);
	}
	break;
    case OBJ_TYP_LEAF_LIST:
	if (obj->def.leaflist) {
	    free_leaflist(obj->def.leaflist, obj->flags);
	}
	break;
    case OBJ_TYP_LIST:
	if (obj->def.list) {
	    free_list(obj->def.list, obj->flags);
	}
	break;
    case OBJ_TYP_CHOICE:
	if (obj->def.choic) {
	    free_choice(obj->def.choic, obj->flags);
	}
	break;
    case OBJ_TYP_CASE:
	if (obj->def.cas) {
	    free_case(obj->def.cas);
	}
	break;
    case OBJ_TYP_USES:
	if (obj->def.uses) {
	    free_uses(obj->def.uses);
	}
	break;
    case OBJ_TYP_REFINE:
	if (obj->def.refine) {
	    free_refine(obj->def.refine);
	}
	break;
    case OBJ_TYP_AUGMENT:
	if (obj->def.augment) {
	    free_augment(obj->def.augment);
	}
	break;
    case OBJ_TYP_RPC:
	if (obj->def.rpc) {
	    free_rpc(obj->def.rpc);
	}
	break;
    case OBJ_TYP_RPCIO:
	if (obj->def.rpcio) {
	    free_rpcio(obj->def.rpcio);
	}
	break;
    case OBJ_TYP_NOTIF:
	if (obj->def.notif) {
	    free_notif(obj->def.notif);
	}
	break;
    case OBJ_TYP_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    m__free(obj);

}  /* obj_free_template */


/********************************************************************
* FUNCTION obj_find_template
* 
* Find an object with the specified name
*
* INPUTS:
*    que == Q of obj_template_t to search
*    modname == module name that defines the obj_template_t
*            == NULL and first match will be done, and the
*               module ignored (Name instead of QName)
*    objname == object name to find
*
* RETURNS:
*    pointer to obj_template_t or NULL if not found in 'que'
*********************************************************************/
obj_template_t *
    obj_find_template (dlq_hdr_t  *que,
		       const xmlChar *modname,
		       const xmlChar *objname)
{

#ifdef DEBUG
    if (!que || !objname) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return find_template(que, modname, objname, FALSE, FALSE, NULL);

}  /* obj_find_template */


/********************************************************************
* FUNCTION obj_find_template_con
* 
* Find an object with the specified name
* Return a const pointer; used by yangdump
*
* INPUTS:
*    que == Q of obj_template_t to search
*    modname == module name that defines the obj_template_t
*            == NULL and first match will be done, and the
*               module ignored (Name instead of QName)
*    objname == object name to find
*
* RETURNS:
*    pointer to obj_template_t or NULL if not found in 'que'
*********************************************************************/
const obj_template_t *
    obj_find_template_con (dlq_hdr_t  *que,
			   const xmlChar *modname,
			   const xmlChar *objname)
{

#ifdef DEBUG
    if (!que || !objname) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return find_template(que, modname, objname, FALSE, FALSE, NULL);

}  /* obj_find_template_con */


/********************************************************************
* FUNCTION obj_find_template_test
* 
* Find an object with the specified name
*
* INPUTS:
*    que == Q of obj_template_t to search
*    modname == module name that defines the obj_template_t
*            == NULL and first match will be done, and the
*               module ignored (Name instead of QName)
*    objname == object name to find
*
* RETURNS:
*    pointer to obj_template_t or NULL if not found in 'que'
*********************************************************************/
obj_template_t *
    obj_find_template_test (dlq_hdr_t  *que,
			    const xmlChar *modname,
			    const xmlChar *objname)
{

#ifdef DEBUG
    if (!que || !objname) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return find_template(que, modname, objname, TRUE, FALSE, NULL);

}  /* obj_find_template_test */


/********************************************************************
* FUNCTION obj_find_template_top
*
* Check if an obj_template_t in the mod->datadefQ or any
* of the include files visible to this module
*
* Top-level access is not tracked, so the 'test' variable
* is hard-wired to FALSE
*
* INPUTS:
*   mod == ncx_module to check
*   modname == module name for the object (needed for augments)
*              (may be NULL to match any 'objname' instance)
*   objname == object name to find
*
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
obj_template_t *
    obj_find_template_top (ncx_module_t *mod,
			   const xmlChar *modname,
			   const xmlChar *objname)
{
    obj_template_t *obj;
    yang_node_t    *node;
    ncx_include_t  *inc;

#ifdef DEBUG
    if (!mod || !objname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    /* check the main module */
    obj = find_template(&mod->datadefQ, 
                        modname, 
			objname, 
                        FALSE, 
                        FALSE, 
                        NULL);
    if (obj) {
	return obj;
    }

    if (!mod->allincQ) {
	return NULL;  /* NCX module */
    }

    /* check all the submodules, but only the ones visible
     * to this module or submodule, YANG only
     */
    for (inc = (ncx_include_t *)dlq_firstEntry(&mod->includeQ);
	 inc != NULL;
	 inc = (ncx_include_t *)dlq_nextEntry(inc)) {

	/* get the real submodule struct */
	if (!inc->submod) {
	    node = yang_find_node(mod->allincQ, 
				  inc->submodule,
				  inc->revision);
	    if (node) {
		inc->submod = node->submod;
	    }
	    if (!inc->submod) {
		/* include not found, should not be in Q !!! */
		SET_ERROR(ERR_INTERNAL_VAL);
		continue;
	    }
	}

	/* check the type Q in this submodule */
	obj = find_template(&inc->submod->datadefQ,
			    modname, 
                            objname,
			    FALSE, 
                            FALSE, 
                            NULL);
	if (obj) {
	    return obj;
	}
    }

    return NULL;

}   /* obj_find_template_top */


/********************************************************************
* FUNCTION obj_find_child
* 
* Find a child object with the specified Qname
*
* !!! This function checks for accessible names only!!!
* !!! That means child nodes of choice->case will be
* !!! present instead of the choice name or case name
*
* INPUTS:
*    obj == obj_template_t to check
*    modname == module name that defines the obj_template_t
*            == NULL and first match will be done, and the
*               module ignored (Name instead of QName)
*    objname == object name to find
*
* RETURNS:
*    pointer to obj_template_t or NULL if not found
*********************************************************************/
obj_template_t *
    obj_find_child (obj_template_t *obj,
		    const xmlChar *modname,
		    const xmlChar *objname)
{
    dlq_hdr_t  *que;

#ifdef DEBUG
    if (!obj || !objname) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    que = obj_get_datadefQ(obj);
    if (que) {
	return find_template(que, 
                             modname, 
			     objname, 
                             TRUE, 
                             FALSE, 
                             NULL);
    }

    return NULL;

}  /* obj_find_child */


/********************************************************************
* FUNCTION obj_find_child_str
* 
* Find a child object with the specified Qname
*
* INPUTS:
*    obj == obj_template_t to check
*    modname == module name that defines the obj_template_t
*            == NULL and first match will be done, and the
*               module ignored (Name instead of QName)
*    objname == object name to find, not Z-terminated
*    objnamelen == length of objname string
*
* RETURNS:
*    pointer to obj_template_t or NULL if not found
*********************************************************************/
obj_template_t *
    obj_find_child_str (obj_template_t *obj,
			const xmlChar *modname,
			const xmlChar *objname,
			uint32 objnamelen)
{
    obj_template_t *template;
    dlq_hdr_t      *que;
    xmlChar              *buff;

#ifdef DEBUG
    if (!obj || !objname) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (objnamelen > NCX_MAX_NLEN) {
	return NULL;
    }
    
    que = obj_get_datadefQ(obj);
    if (que) {
	buff = m__getMem(objnamelen+1);
	if (buff) {
	    xml_strncpy(buff, objname, objnamelen);
	    template = find_template(que, 
				     modname, 
				     buff, 
				     TRUE, 
				     FALSE, 
				     NULL);
	    m__free(buff);
	    return template;
	}
    }

    return NULL;

}  /* obj_find_child_str */


/********************************************************************
* FUNCTION obj_match_child_str
* 
* Match a child object with the specified Qname
* Find first command that matches all N chars of objname
*
* !!! This function checks for accessible names only!!!
* !!! That means child nodes of choice->case will be
* !!! present instead of the choice name or case name
*
* INPUTS:
*    obj == obj_template_t to check
*    modname == module name that defines the obj_template_t
*            == NULL and first match will be done, and the
*               module ignored (Name instead of QName)
*    objname == object name to find, not Z-terminated
*    objnamelen == length of objname string
*    matchcount == address of return parameter match count
*                  (may be NULL)
* OUTPUTS:
*   if non-NULL:
*    *matchcount == number of parameters that matched
*                   only the first match will be returned
*
* RETURNS:
*    pointer to obj_template_t or NULL if not found
*********************************************************************/
obj_template_t *
    obj_match_child_str (obj_template_t *obj,
			 const xmlChar *modname,
			 const xmlChar *objname,
			 uint32 objnamelen,
			 uint32 *matchcount)
{
    obj_template_t  *template;
    dlq_hdr_t       *que;
    xmlChar               *buff;

#ifdef DEBUG
    if (!obj || !objname) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (objnamelen > NCX_MAX_NLEN) {
	return NULL;
    }
    
    que = obj_get_datadefQ(obj);
    if (que) {
	buff = m__getMem(objnamelen+1);
	if (buff) {
	    xml_strncpy(buff, objname, objnamelen);
	    template = find_template(que, 
				     modname, 
				     buff, 
				     TRUE, 
				     TRUE, 
				     matchcount);
	    m__free(buff);
	    return template;
	}
    }

    return NULL;

}  /* obj_match_child_str */


/********************************************************************
* FUNCTION obj_first_child
* 
* Get the first child object if the specified object
* has any children
*
*  !!!! SKIPS OVER AUGMENT AND USES !!!!
*
* INPUTS:
*    obj == obj_template_t to check

* RETURNS:
*    pointer to first child obj_template_t or 
*    NULL if not found 
*********************************************************************/
obj_template_t *
    obj_first_child (obj_template_t *obj)
{
    dlq_hdr_t       *que;
    obj_template_t  *chobj;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    que = obj_get_datadefQ(obj);
    if (que) {
	for (chobj = (obj_template_t *)dlq_firstEntry(que);
	     chobj != NULL;
	     chobj = (obj_template_t *)dlq_nextEntry(chobj)) {
	    if (obj_has_name(chobj) && obj_is_enabled(chobj)) {
		return chobj;
	    }
	}
    }

    return NULL;

}  /* obj_first_child */


/********************************************************************
* FUNCTION obj_last_child
* 
* Get the last child object if the specified object
* has any children
*
*  !!!! SKIPS OVER AUGMENT AND USES !!!!
*
* INPUTS:
*    obj == obj_template_t to check

* RETURNS:
*    pointer to first child obj_template_t or 
*    NULL if not found 
*********************************************************************/
obj_template_t *
    obj_last_child (obj_template_t *obj)
{
    dlq_hdr_t       *que;
    obj_template_t  *chobj;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    que = obj_get_datadefQ(obj);
    if (que) {
	for (chobj = (obj_template_t *)dlq_lastEntry(que);
	     chobj != NULL;
	     chobj = (obj_template_t *)dlq_prevEntry(chobj)) {
	    if (obj_has_name(chobj) && obj_is_enabled(chobj)) {
		return chobj;
	    }
	}
    }

    return NULL;

}  /* obj_last_child */


/********************************************************************
* FUNCTION obj_next_child
* 
* Get the next child object if the specified object
* has any children
*
*  !!!! SKIPS OVER AUGMENT AND USES !!!!
*
* INPUTS:
*    obj == obj_template_t to check

* RETURNS:
*    pointer to next child obj_template_t or 
*    NULL if not found 
*********************************************************************/
obj_template_t *
    obj_next_child (obj_template_t *obj)
{
    obj_template_t  *next;
    boolean          done;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    next = obj;
    done = FALSE;
    while (!done) {
	next = (obj_template_t *)dlq_nextEntry(next);
	if (!next) {
	    done = TRUE;
	} else if (obj_has_name(next) && obj_is_enabled(next)) {
	    return next;
	}
    }
    return NULL;

}  /* obj_next_child */


/********************************************************************
* FUNCTION obj_previous_child
* 
* Get the previous child object if the specified object
* has any children
*
*  !!!! SKIPS OVER AUGMENT AND USES !!!!
*
* INPUTS:
*    obj == obj_template_t to check

* RETURNS:
*    pointer to next child obj_template_t or 
*    NULL if not found 
*********************************************************************/
obj_template_t *
    obj_previous_child (obj_template_t *obj)
{
    obj_template_t  *prev;
    boolean          done;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    prev = obj;
    done = FALSE;
    while (!done) {
	prev = (obj_template_t *)dlq_prevEntry(prev);
	if (!prev) {
	    done = TRUE;
	} else if (obj_has_name(prev) && obj_is_enabled(prev)) {
	    return prev;
	}
    }
    return NULL;

}  /* obj_previous_child */


/********************************************************************
* FUNCTION obj_first_child_deep
* 
* Get the first child object if the specified object
* has any children.  Look past choices and cases to
* the real nodes within them
*
*  !!!! SKIPS OVER AUGMENT AND USES AND CHOICES AND CASES !!!!
*
* INPUTS:
*    obj == obj_template_t to check

* RETURNS:
*    pointer to first child obj_template_t or 
*    NULL if not found 
*********************************************************************/
obj_template_t *
    obj_first_child_deep (obj_template_t *obj)
{
    dlq_hdr_t       *que;
    obj_template_t  *chobj;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    /* go through the child nodes of this object looking
     * for the first data object; skip over all meta-objects
     */
    que = obj_get_datadefQ(obj);
    if (que) {
	for (chobj = (obj_template_t *)dlq_firstEntry(que);
	     chobj != NULL;
	     chobj = (obj_template_t *)dlq_nextEntry(chobj)) {

	    if (obj_has_name(chobj) && obj_is_enabled(chobj)) {
		if (chobj->objtype == OBJ_TYP_CHOICE ||
		    chobj->objtype == OBJ_TYP_CASE) {
		    return (obj_first_child_deep(chobj));
		} else {
		    return chobj;
		}
	    }
	}
    }

    return NULL;

}  /* obj_first_child_deep */


/********************************************************************
* FUNCTION obj_next_child_deep
* 
* Get the next child object if the specified object
* has any children.  Look past choice and case nodes
* to the real nodes within them
*
*  !!!! SKIPS OVER AUGMENT AND USES !!!!
*
* INPUTS:
*    obj == obj_template_t to check
*
* RETURNS:
*    pointer to next child obj_template_t or 
*    NULL if not found 
*********************************************************************/
obj_template_t *
    obj_next_child_deep (obj_template_t *obj)
{
    obj_template_t  *cas, *next, *last, *child;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    /* start the loop at the current object to set the
     * 'last' object correctly
     */
    next = obj;
    while (next) {
	last = next;

	/* try next sibling */
	next = obj_next_child(next);
	if (next) {
	    switch (next->objtype) {
	    case OBJ_TYP_CHOICE:
		/* dive into each case to find a first object
		 * this should return the first object in the 
		 * first case, but it checks the entire choice
		 * to support empty case arms
		 */
		for (cas = obj_first_child(next);
		     cas != NULL;
		     cas = obj_next_child(next)) {
		    child = obj_first_child(cas);
		    if (child) {
			return child;
		    }
		}
		continue;
	    case OBJ_TYP_CASE:
		child = obj_first_child(next);
		if (child) {
		    return child;
		}
		continue;
	    default:
		return next;
	    }
	}

	/* was last sibling, try parent if this is a case */
	if (last->parent && 
	    (last->parent->objtype==OBJ_TYP_CASE)) {

 	    cas = (obj_template_t *)
		dlq_nextEntry(last->parent);
	    if (!cas) {
		/* no next case, try next object after choice */
		return obj_next_child_deep(last->parent->parent);
	    } else {
		/* keep trying the next case until one with
		 * a child node is found
		 */
		while (1) {
		    next = obj_first_child(cas);
		    if (next) {
			return next;
		    } else {
			cas = (obj_template_t *)
			    dlq_nextEntry(cas);
			if (!cas) {
			    /* no next case, ret. object after choice */
			    return 
				obj_next_child_deep(last->parent->parent);
			}
		    }
		}
		/*NOTREACHED*/
	    }
	}
    }
    return NULL;

}  /* obj_next_child_deep */


/********************************************************************
* FUNCTION obj_find_all_children
* 
* Find all occurances of the specified node(s)
* within the children of the current node. 
* The walker fn will be called for each match.  
*
* If the walker function returns TRUE, then the 
* walk will continue; If FALSE it will terminate right away
*
* This function skips choice and case nodes and
* only processes real data nodes
*
* INPUTS:
*    exprmod == module containing XPath expression
*    walkerfn == callback function to use
*    cookie1 == cookie1 value to pass to walker fn
*    cookie2 == cookie2 value to pass to walker fn
*    startnode == start node to check
*    modname == module name; 
*                only matches in this module namespace
*                will be returned
*            == NULL:
*                 namespace matching will be skipped
*    childname == name of child node to find
*              == NULL to match any child name
*    configonly == TRUE to skip over non-config nodes
*                  FALSE to check all nodes
*                  Only used if childname == NULL
*    textmode == TRUE if just testing for text() nodes
*                name and modname will be ignored in this mode
*                FALSE if using name and modname to filter
*    useroot == TRUE is it is safe to use the toproot
*               FALSE if not, use all moduleQ search instead
*
* RETURNS:
*   TRUE if normal termination occurred
*   FALSE if walker fn requested early termination
*********************************************************************/
boolean
    obj_find_all_children (ncx_module_t *exprmod,
			   obj_walker_fn_t walkerfn,
			   void *cookie1,
			   void *cookie2,
			   obj_template_t *startnode,
			   const xmlChar *modname,
			   const xmlChar *childname,
			   boolean configonly,
			   boolean textmode,
			   boolean useroot)
{
    dlq_hdr_t         *datadefQ;
    obj_template_t    *obj;
    ncx_module_t      *mod;
    boolean            fnresult;

#ifdef DEBUG
    if (!exprmod || !walkerfn || !startnode) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    if (obj_is_root(startnode) && !useroot) {

	for (obj = ncx_get_first_data_object(exprmod);
	     obj != NULL;
	     obj = ncx_get_next_data_object(exprmod, obj)) {

	    fnresult = test_one_child(exprmod,
				      walkerfn,
				      cookie1,
				      cookie2,
				      obj,
				      modname,
				      childname,
				      configonly,
				      textmode);
	    if (!fnresult) {
		return FALSE;
	    }
	}

	for (mod = ncx_get_first_module();
	     mod != NULL;
	     mod = ncx_get_next_module(mod)) {

	    for (obj = ncx_get_first_data_object(mod);
		 obj != NULL;
		 obj = ncx_get_next_data_object(mod, obj)) {

		fnresult = test_one_child(exprmod,
					  walkerfn,
					  cookie1,
					  cookie2,
					  obj,
					  modname,
					  childname,
					  configonly,
					  textmode);
		if (!fnresult) {
		    return FALSE;
		}
	    }
	}

	for (mod = ncx_get_first_session_module();
	     mod != NULL;
	     mod = ncx_get_next_session_module(mod)) {

	    for (obj = ncx_get_first_data_object(mod);
		 obj != NULL;
		 obj = ncx_get_next_data_object(mod, obj)) {

		fnresult = test_one_child(exprmod,
					  walkerfn,
					  cookie1,
					  cookie2,
					  obj,
					  modname,
					  childname,
					  configonly,
					  textmode);
		if (!fnresult) {
		    return FALSE;
		}
	    }
	}
    } else {

	datadefQ = obj_get_datadefQ(startnode);
	if (!datadefQ) {
	    return TRUE;
	}

	for (obj = (obj_template_t *)dlq_firstEntry(datadefQ);
	     obj != NULL;
	     obj = (obj_template_t *)dlq_nextEntry(obj)) {

	    fnresult = test_one_child(exprmod,
				      walkerfn,
				      cookie1,
				      cookie2,
				      obj,
				      modname,
				      childname,
				      configonly,
				      textmode);
	    if (!fnresult) {
		return FALSE;
	    }
	}
    }

    return TRUE;

}  /* obj_find_all_children */


/********************************************************************
* FUNCTION obj_find_all_ancestors
* 
* Find all occurances of the specified node(s)
* within the ancestors of the current node. 
* The walker fn will be called for each match.  
*
* If the walker function returns TRUE, then the 
* walk will continue; If FALSE it will terminate right away
*
* This function skips choice and case nodes and
* only processes real data nodes
*
* INPUTS:
*    exprmod == module containing XPath object
*    walkerfn == callback function to use
*    cookie1 == cookie1 value to pass to walker fn
*    cookie2 == cookie2 value to pass to walker fn
*    startnode == start node to check
*    modname == module name; 
*                only matches in this module namespace
*                will be returned
*            == NULL:
*                 namespace matching will be skipped
*    name == name of ancestor node to find
*              == NULL to match any ancestor name
*    configonly == TRUE to skip over non-config nodes
*                  FALSE to check all nodes
*                  Only used if name == NULL
*    textmode == TRUE if just testing for text() nodes
*                name and modname will be ignored in this mode
*                FALSE if using name and modname to filter
*    useroot == TRUE is it is safe to use the toproot
*               FALSE if not, use all moduleQ search instead
*    orself == TRUE if axis is really ancestor-or-self
*              FALSE if axis is ancestor
*    fncalled == address of return function called flag
*
* OUTPUTS:
*   *fncalled set to TRUE if a callback function was called
*
* RETURNS:
*   TRUE if normal termination occurred
*   FALSE if walker fn requested early termination
*********************************************************************/
boolean
    obj_find_all_ancestors (ncx_module_t *exprmod,
			    obj_walker_fn_t walkerfn,
			    void *cookie1,
			    void *cookie2,
			    obj_template_t *startnode,
			    const xmlChar *modname,
			    const xmlChar *name,
			    boolean configonly,
			    boolean textmode,
			    boolean useroot,
			    boolean orself,
			    boolean *fncalled)
{
    obj_template_t       *obj;
    ncx_module_t         *mod;
    boolean               fnresult;

#ifdef DEBUG
    if (!exprmod || !walkerfn || !startnode || !fncalled) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    *fncalled = FALSE;

    if (orself) {
	obj = startnode;
    } else {
	obj = startnode->parent;
    }

    if (obj && obj_is_root(obj) && !useroot) {

	for (obj = ncx_get_first_data_object(exprmod);
	     obj != NULL;
	     obj = ncx_get_next_data_object(exprmod, obj)) {

	    fnresult = test_one_ancestor(exprmod,
					 walkerfn,
					 cookie1,
					 cookie2,
					 obj,
					 modname,
					 name,
					 configonly,
					 textmode,
					 orself,
					 fncalled);
	    if (!fnresult) {
		return FALSE;
	    }
	}

	for (mod = ncx_get_first_module();
	     mod != NULL;
	     mod = ncx_get_next_module(mod)) {

	    for (obj = ncx_get_first_data_object(mod);
		 obj != NULL;
		 obj = ncx_get_next_data_object(mod, obj)) {

		fnresult = test_one_ancestor(exprmod,
					     walkerfn,
					     cookie1,
					     cookie2,
					     obj,
					     modname,
					     name,
					     configonly,
					     textmode,
					     orself,
					     fncalled);
		if (!fnresult) {
		    return FALSE;
		}
	    }
	}

	for (mod = ncx_get_first_session_module();
	     mod != NULL;
	     mod = ncx_get_next_session_module(mod)) {

	    for (obj = ncx_get_first_data_object(mod);
		 obj != NULL;
		 obj = ncx_get_next_data_object(mod, obj)) {

		fnresult = test_one_ancestor(exprmod,
					     walkerfn,
					     cookie1,
					     cookie2,
					     obj,
					     modname,
					     name,
					     configonly,
					     textmode,
					     orself,
					     fncalled);
		if (!fnresult) {
		    return FALSE;
		}
	    }
	}
    } else {
	while (obj) {
	    if (obj->objtype == OBJ_TYP_CHOICE ||
		obj->objtype == OBJ_TYP_CASE) {
		fnresult = TRUE;
	    } else {
		fnresult = process_one_walker_child(walkerfn,
						    cookie1,
						    cookie2,
						    obj,
						    modname, 
						    name,
						    configonly,
						    textmode,
						    fncalled);
	    }
	    if (!fnresult) {
		return FALSE;
	    }
	    obj = obj->parent;
	}
    }

    return TRUE;

}  /* obj_find_all_ancestors */


/********************************************************************
* FUNCTION obj_find_all_descendants
* 
* Find all occurances of the specified node(s)
* within the descendants of the current node. 
* The walker fn will be called for each match.  
*
* If the walker function returns TRUE, then the 
* walk will continue; If FALSE it will terminate right away
*
* This function skips choice and case nodes and
* only processes real data nodes
*
* INPUTS:
*    exprmod == module containing XPath expression
*    walkerfn == callback function to use
*    cookie1 == cookie1 value to pass to walker fn
*    cookie2 == cookie2 value to pass to walker fn
*    startnode == start node to check
*    modname == module name; 
*                only matches in this module namespace
*                will be returned
*            == NULL:
*                 namespace matching will be skipped
*    name == name of descendant node to find
*              == NULL to match any descendant name
*    configonly == TRUE to skip over non-config nodes
*                  FALSE to check all nodes
*                  Only used if name == NULL
*    textmode == TRUE if just testing for text() nodes
*                name and modname will be ignored in this mode
*                FALSE if using name and modname to filter
*    useroot == TRUE is it is safe to use the toproot
*               FALSE if not, use all moduleQ search instead
*    orself == TRUE if axis is really ancestor-or-self
*              FALSE if axis is ancestor
*    fncalled == address of return function called flag
*
* OUTPUTS:
*   *fncalled set to TRUE if a callback function was called
*
* RETURNS:
*   TRUE if normal termination occurred
*   FALSE if walker fn requested early termination
*********************************************************************/
boolean
    obj_find_all_descendants (ncx_module_t *exprmod,
			      obj_walker_fn_t walkerfn,
			      void *cookie1,
			      void *cookie2,
			      obj_template_t *startnode,
			      const xmlChar *modname,
			      const xmlChar *name,
			      boolean configonly,
			      boolean textmode,
			      boolean useroot,
			      boolean orself,
			      boolean *fncalled)
{
    obj_template_t       *obj;
    ncx_module_t         *mod;
    boolean               fnresult;

#ifdef DEBUG
    if (!exprmod || !walkerfn || !startnode || !fncalled) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    *fncalled = FALSE;

    if (obj_is_root(startnode) && !useroot) {

	for (obj = ncx_get_first_data_object(exprmod);
	     obj != NULL;
	     obj = ncx_get_next_data_object(exprmod, obj)) {

	    fnresult = test_one_descendant(exprmod,
					   walkerfn,
					   cookie1,
					   cookie2,
					   obj,
					   modname,
					   name,
					   configonly,
					   textmode,
					   orself,
					   fncalled);
	    if (!fnresult) {
		return FALSE;
	    }
	}

	for (mod = ncx_get_first_module();
	     mod != NULL;
	     mod = ncx_get_next_module(mod)) {

	    for (obj = ncx_get_first_data_object(mod);
		 obj != NULL;
		 obj = ncx_get_next_data_object(mod, obj)) {

		fnresult = test_one_descendant(exprmod,
					       walkerfn,
					       cookie1,
					       cookie2,
					       obj,
					       modname,
					       name,
					       configonly,
					       textmode,
					       orself,
					       fncalled);
		if (!fnresult) {
		    return FALSE;
		}
	    }
	}

	for (mod = ncx_get_first_session_module();
	     mod != NULL;
	     mod = ncx_get_next_session_module(mod)) {

	    for (obj = ncx_get_first_data_object(mod);
		 obj != NULL;
		 obj = ncx_get_next_data_object(mod, obj)) {

		fnresult = test_one_descendant(exprmod,
					       walkerfn,
					       cookie1,
					       cookie2,
					       obj,
					       modname,
					       name,
					       configonly,
					       textmode,
					       orself,
					       fncalled);
		if (!fnresult) {
		    return FALSE;
		}
	    }
	}
    } else {
	fnresult = test_one_descendant(exprmod,
				       walkerfn,
				       cookie1,
				       cookie2,
				       startnode,
				       modname,
				       name,
				       configonly,
				       textmode,
				       orself,
				       fncalled);
	if (!fnresult) {
	    return FALSE;
	}
    }
    return TRUE;

}  /* obj_find_all_descendants */


/********************************************************************
* FUNCTION obj_find_all_pfaxis
* 
* Find all occurances of the specified preceding
* or following node(s).  Could also be
* within the descendants of the current node. 
* The walker fn will be called for each match.  
*
* If the walker function returns TRUE, then the 
* walk will continue; If FALSE it will terminate right away
*
* This function skips choice and case nodes and
* only processes real data nodes
*
* INPUTS:
*    exprmod == module containing object
*    walkerfn == callback function to use
*    cookie1 == cookie1 value to pass to walker fn
*    cookie2 == cookie2 value to pass to walker fn
*    startnode == starting sibling node to check
*    modname == module name; 
*                only matches in this module namespace
*                will be returned
*            == NULL:
*                 namespace matching will be skipped
*
*    name == name of preceding or following node to find
*         == NULL to match any name
*    configonly == TRUE to skip over non-config nodes
*                  FALSE to check all nodes
*                  Only used if name == NULL
*    dblslash == TRUE if all decendents of the preceding
*                 or following nodes should be checked
*                FALSE only 1 level is checked
*    textmode == TRUE if just testing for text() nodes
*                name and modname will be ignored in this mode
*                FALSE if using name and modname to filter
*    axis == axis enum to use
*    fncalled == address of return function called flag
*
* OUTPUTS:
*   *fncalled set to TRUE if a callback function was called
*
* RETURNS:
*   TRUE if normal termination occurred
*   FALSE if walker fn requested early termination
*********************************************************************/
boolean
    obj_find_all_pfaxis (ncx_module_t *exprmod,
			 obj_walker_fn_t walkerfn,
			 void *cookie1,
			 void *cookie2,
			 obj_template_t *startnode,
			 const xmlChar *modname,
			 const xmlChar *name,
			 boolean configonly,
			 boolean dblslash,
			 boolean textmode,
			 boolean useroot,
			 ncx_xpath_axis_t axis,
			 boolean *fncalled)
{
    obj_template_t       *obj;
    ncx_module_t         *mod;
    boolean               fnresult, forward;

#ifdef DEBUG
    if (!exprmod || !walkerfn || !startnode || !fncalled) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    *fncalled = FALSE;

    /* check the Q containing the startnode
     * for preceding or following nodes;
     * could be sibling node check or any node check
     */
    switch (axis) {
    case XP_AX_PRECEDING:
	dblslash = TRUE;
	/* fall through */
    case XP_AX_PRECEDING_SIBLING:
	/* execute the callback for all preceding nodes
	 * that match the filter criteria 
	 */
	forward = FALSE;
	break;
    case XP_AX_FOLLOWING:
	dblslash = TRUE;
	/* fall through */
    case XP_AX_FOLLOWING_SIBLING:
	forward = TRUE;
	break;
    case XP_AX_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

    if (obj_is_root(startnode) && !dblslash) {
	return TRUE;
    }

    if (obj_is_root(startnode) && !useroot) {

	for (obj = ncx_get_first_data_object(exprmod);
	     obj != NULL;
	     obj = ncx_get_next_data_object(exprmod, obj)) {

	    fnresult = test_one_pfnode(exprmod,
				       walkerfn,
				       cookie1,
				       cookie2,
				       obj,
				       modname,
				       name,
				       configonly,
				       dblslash,
				       textmode,
				       forward,
				       axis,
				       fncalled);
	    if (!fnresult) {
		return FALSE;
	    }
	}

	for (mod = ncx_get_first_module();
	     mod != NULL;
	     mod = ncx_get_next_module(mod)) {

	    for (obj = ncx_get_first_data_object(mod);
		 obj != NULL;
		 obj = ncx_get_next_data_object(mod, obj)) {

		fnresult = test_one_pfnode(exprmod,
					   walkerfn,
					   cookie1,
					   cookie2,
					   obj,
					   modname,
					   name,
					   configonly,
					   dblslash,
					   textmode,
					   forward,
					   axis,
					   fncalled);
		if (!fnresult) {
		    return FALSE;
		}
	    }
	}

	for (mod = ncx_get_first_session_module();
	     mod != NULL;
	     mod = ncx_get_next_session_module(mod)) {

	    for (obj = ncx_get_first_data_object(mod);
		 obj != NULL;
		 obj = ncx_get_next_data_object(mod, obj)) {

		fnresult = test_one_pfnode(exprmod,
					   walkerfn,
					   cookie1,
					   cookie2,
					   obj,
					   modname,
					   name,
					   configonly,
					   dblslash,
					   textmode,
					   forward,
					   axis,
					   fncalled);
		if (!fnresult) {
		    return FALSE;
		}
	    }
	}
    } else {
	fnresult = test_one_pfnode(exprmod,
				   walkerfn,
				   cookie1,
				   cookie2,
				   startnode,
				   modname,
				   name,
				   configonly,
				   dblslash,
				   textmode,
				   forward,
				   axis,
				   fncalled);
	if (!fnresult) {
	    return FALSE;
	}
    }

    return TRUE;

}  /* obj_find_all_pfaxis */


/********************************************************************
* FUNCTION obj_find_case
* 
* Find a specified case arm by name
*
* INPUTS:
*    choic == choice struct to check
*    modname == name of the module that added this case (may be NULL)
*    casname == name of the case to find
*
* RETURNS:
*    pointer to obj_case_t for requested case, NULL if not found
*********************************************************************/
obj_case_t *
    obj_find_case (obj_choice_t *choic,
		   const xmlChar *modname,
		   const xmlChar *casname)
{
    obj_template_t *casobj;
    obj_case_t     *cas;

#ifdef DEBUG
    if (!choic || !casname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (casobj = (obj_template_t *)dlq_firstEntry(choic->caseQ);
	 casobj != NULL;
	 casobj = (obj_template_t *)dlq_nextEntry(casobj)) {

	cas = casobj->def.cas;
	if (modname && xml_strcmp(obj_get_mod_name(casobj), modname)) {
	    continue;
	}

	if (!xml_strcmp(casname, cas->name)) {
	    return cas;
	}
    }
    return NULL;

}  /* obj_find_case */


/********************************************************************
* FUNCTION obj_new_rpcio
* 
* Malloc and initialize the fields in a an obj_rpcio_t
* Fields are setup within the new obj_template_t, based
* on the values in rpcobj
*
* INPUTS:
*   rpcobj == parent OBJ_TYP_RPC template
*   name == name string of the node (input or output)
*
* RETURNS:
*    pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
obj_template_t * 
    obj_new_rpcio (obj_template_t *rpcobj,
		   const xmlChar *name)
{
    obj_template_t  *rpcio;

#ifdef DEBUG
    if (!rpcobj || !name) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    rpcio = obj_new_template(OBJ_TYP_RPCIO);
    if (!rpcio) {
	return NULL;
    }
    rpcio->def.rpcio->name = xml_strdup(name);
    if (!rpcio->def.rpcio->name) {
	obj_free_template(rpcio);
	return NULL;
    }
    ncx_set_error(&rpcio->tkerr,
                  rpcobj->tkerr.mod,
                  rpcobj->tkerr.linenum,
                  rpcobj->tkerr.linepos);

    rpcio->parent = rpcobj;

    return rpcio;

}  /* obj_new_rpcio */


/********************************************************************
* FUNCTION obj_clean_datadefQ
* 
* Clean and free all the obj_template_t structs in the specified Q
*
* INPUTS:
*    datadefQ == Q of obj_template_t to clean
*********************************************************************/
void
    obj_clean_datadefQ (dlq_hdr_t *que)
{
    obj_template_t *obj;

#ifdef DEBUG
    if (!que) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    while (!dlq_empty(que)) {
	obj = (obj_template_t *)dlq_deque(que);
	obj_free_template(obj);
    }

}  /* obj_clean_datadefQ */


/********************************************************************
* FUNCTION obj_find_type
*
* Check if a typ_template_t in the obj typedefQ hierarchy
*
* INPUTS:
*   obj == obj_template using the typedef
*   typname == type name to find
*
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
typ_template_t *
    obj_find_type (obj_template_t *obj,
		   const xmlChar *typname)
{
    dlq_hdr_t      *que;
    typ_template_t *typ;
    grp_template_t *testgrp;

#ifdef DEBUG
    if (!obj || !typname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    /* if this is a direct child of a grouping, try the tryedefQ
     * in the grouping first
     */
    if (obj->grp) {
	que = &obj->grp->typedefQ;
	typ = ncx_find_type_que(que, typname);
	if (typ) {
	    return typ;
	}

	testgrp = obj->grp->parentgrp;
	while (testgrp) {
	    typ = ncx_find_type_que(&testgrp->typedefQ, typname);
	    if (typ) {
		return typ;
	    }
	    testgrp = testgrp->parentgrp;
	}
    }

    /* object not in directly in a group or nothing found
     * check if this object has a typedefQ
     */
    que = NULL;

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	que = obj->def.container->typedefQ;
	break;
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
	break;
    case OBJ_TYP_LIST:
	que = obj->def.list->typedefQ;
	break;
    case OBJ_TYP_CHOICE:
    case OBJ_TYP_CASE:
    case OBJ_TYP_USES:
    case OBJ_TYP_REFINE:
    case OBJ_TYP_AUGMENT:
	break;
    case OBJ_TYP_RPC:
	que = &obj->def.rpc->typedefQ;
	break;
    case OBJ_TYP_RPCIO:
	que = &obj->def.rpcio->typedefQ;
	break;
    case OBJ_TYP_NOTIF:
	que = &obj->def.notif->typedefQ;
	break;
    case OBJ_TYP_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

    if (que) {
	typ = ncx_find_type_que(que, typname);
	if (typ) {
	    return typ;
	}
    }

    if (obj->parent && !obj_is_root(obj->parent)) {
	return obj_find_type(obj->parent, typname);
    } else {
	return NULL;
    }

}   /* obj_find_type */


/********************************************************************
* FUNCTION obj_find_grouping
*
* Check if a grp_template_t in the obj groupingQ hierarchy
*
* INPUTS:
*   obj == obj_template using the grouping
*   grpname == grouping name to find
*
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
grp_template_t *
    obj_find_grouping (obj_template_t *obj,
		       const xmlChar *grpname)
{
    dlq_hdr_t      *que;
    grp_template_t *grp, *testgrp;

#ifdef DEBUG
    if (!obj || !grpname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    /* check direct nesting within a grouping chain */
    if (obj->grp) {
	grp = ncx_find_grouping_que(&obj->grp->groupingQ, grpname);
	if (grp) {
	    return grp;
	}

	testgrp = obj->grp->parentgrp;
	while (testgrp) {
	    if (!xml_strcmp(testgrp->name, grpname)) {
		return testgrp;
	    } else {
		grp = ncx_find_grouping_que(&testgrp->groupingQ, grpname);
		if (grp) {
		    return grp;
		}
	    }
	    testgrp = testgrp->parentgrp;
	}
    }

    /* check the object has a groupingQ within the object chain */
    que = NULL;

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	que = obj->def.container->groupingQ;
	break;
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
	break;
    case OBJ_TYP_LIST:
	que = obj->def.list->groupingQ;
	break;
    case OBJ_TYP_CHOICE:
    case OBJ_TYP_CASE:
    case OBJ_TYP_USES:
    case OBJ_TYP_REFINE:
    case OBJ_TYP_AUGMENT:
	break;
    case OBJ_TYP_RPC:
	que = &obj->def.rpc->groupingQ;
	break;
    case OBJ_TYP_RPCIO:
	que = &obj->def.rpcio->groupingQ;
	break;
    case OBJ_TYP_NOTIF:
	que = &obj->def.notif->groupingQ;
	break;
    case OBJ_TYP_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

    if (que) {
	grp = ncx_find_grouping_que(que, grpname);
	if (grp) {
	    return grp;
	}
    }

    if (obj->parent && !obj_is_root(obj->parent)) {
	return obj_find_grouping(obj->parent, grpname);
    } else {
	return NULL;
    }

}   /* obj_find_grouping */


/********************************************************************
* FUNCTION obj_set_named_type
* 
* Resolve type test 
* Called during phase 2 of module parsing
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   typname == name field from typ->name  (may be NULL)
*   typdef == typdef in progress
*   parent == obj_template containing this typedef
*          == NULL if this is the top-level, use mod->typeQ
*   grp == grp_template containing this typedef
*          == NULL if the typedef is not contained in a grouping
*
* RETURNS:
*   status
*********************************************************************/
status_t 
    obj_set_named_type (tk_chain_t *tkc,
			ncx_module_t *mod,
			const xmlChar *typname,
			typ_def_t *typdef,
			obj_template_t *parent,
			grp_template_t *grp)
{
    typ_template_t *testtyp;

    if (typdef->class == NCX_CL_NAMED &&
	typdef->def.named.typ==NULL) {

	/* assumed to be a named type from this module
	 * because any named type from another module
	 * would get resolved OK, or fail due to syntax
	 * or dependency loop
	 */
	if (typname && !xml_strcmp(typname, typdef->typename)) {
	    log_error("\nError: typedef '%s' cannot use type '%s'",
		      typname, typname);
	    tkc->curerr = &typdef->tkerr;
	    return ERR_NCX_DEF_LOOP;
	}

	testtyp = NULL;

	/* find the type within the specified typedef Q */
	if (typdef->typename) {
	    if (grp) {
		testtyp = find_type_in_grpchain(grp, typdef->typename);
	    }

	    if (!testtyp && parent) {
		testtyp = obj_find_type(parent, typdef->typename);
	    }

	    if (!testtyp) {
		testtyp = ncx_find_type(mod, typdef->typename);
	    }
	}

	if (!testtyp) {
	    log_error("\nError: type '%s' not found", typdef->typename);
	    tkc->curerr = &typdef->tkerr;
	    return ERR_NCX_UNKNOWN_TYPE;
	} else {
	    typdef->def.named.typ = testtyp;
	    typdef->linenum = testtyp->tkerr.linenum;
	    testtyp->used = TRUE;
	}
    }
    return NO_ERR;

}   /* obj_set_named_type */


/********************************************************************
* FUNCTION obj_clone_template
*
* Clone an obj_template_t
* Copy the pointers from the srcobj into the new obj
*
* If the mobj is non-NULL, then the non-NULL revisable
* fields in the mobj struct will be merged into the new object
*
* INPUTS:
*   mod == module struct that is defining the new cloned data
*          this may be different than the module that will
*          contain the cloned data (except top-level objects)
*   srcobj == obj_template to clone
*             !!! This struct MUST NOT be deleted!!!
*             !!! Unless all of its clones are also deleted !!!
*   mobjQ == merge object Q (may be NULL)
*           datadefQ to check for OBJ_TYP_REFINE nodes
*           If the target of the refine node matches the
*           srcobj (e.g., from same grouping), then the
*           sub-clauses in that refinement-stmt that
*           are allowed to be revised will be checked
*
* RETURNS:
*   pointer to malloced clone obj_template_t
*   NULL if malloc failer error or internal error
*********************************************************************/
obj_template_t *
    obj_clone_template (ncx_module_t *mod,
			obj_template_t *srcobj,
			dlq_hdr_t *mobjQ)
{
    obj_template_t     *newobj, *mobj, *testobj;
    status_t            res;

#ifdef DEBUG
    if (!srcobj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
    if (srcobj->objtype == OBJ_TYP_NONE ||
	srcobj->objtype > OBJ_TYP_AUGMENT) {
        SET_ERROR(ERR_INTERNAL_VAL);
        return NULL;
    }
#endif

    newobj = new_blank_template();
    if (!newobj) {
	return NULL;
    }

    if (srcobj->when) {
        newobj->when = xpath_clone_pcb(srcobj->when);
        if (newobj->when == NULL) {
            obj_free_template(newobj);
            return NULL;
        }
    }

    /* set most of the common fields but leave the mod and parent NULL
     * since the uses or augment calling this fn is going to
     * re-prent the cloned node under a different part of the tree
     */
    newobj->objtype = srcobj->objtype;
    ncx_set_error(&newobj->tkerr,
                  srcobj->tkerr.mod,
                  srcobj->tkerr.linenum,
                  srcobj->tkerr.linepos);
    newobj->flags = (srcobj->flags | OBJ_FL_CLONE);
    newobj->nsid = mod->nsid;

    /* do not set the group in a clone */
    /* newobj->grp = srcobj->grp; */

    mobj = NULL;
    if (mobjQ) {
	for (testobj = (obj_template_t *)dlq_firstEntry(mobjQ);
	     testobj != NULL && mobj == NULL;
	     testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

	    if (testobj->objtype != OBJ_TYP_REFINE) {
		continue;
	    }

	    if (testobj->def.refine->targobj == srcobj) {
		mobj = testobj;
	    }
	}
    }

    if (mobj) {
	newobj->flags |= mobj->flags;
    }

    res = clone_appinfoQ(&newobj->appinfoQ,
			 &srcobj->appinfoQ,
			 (mobj) ? &mobj->appinfoQ : NULL);
    if (res != NO_ERR) {
	free_blank_template(newobj);
	return NULL;
    }
    
    /* set the specific object definition type */
    switch (srcobj->objtype) {
    case OBJ_TYP_CONTAINER:
	newobj->def.container = 
	    clone_container(mod, newobj, 
			    srcobj->def.container,
			    (mobj) ? mobj->def.refine : NULL,
			    mobjQ);
	if (!newobj->def.container) {
	    res = ERR_INTERNAL_MEM;
	}
	break;
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_LEAF:
	newobj->def.leaf = 
	    clone_leaf(srcobj->def.leaf,
		       (mobj) ? mobj->def.refine : NULL);
	if (!newobj->def.leaf) {
	    res = ERR_INTERNAL_MEM;
	}
	break;
    case OBJ_TYP_LEAF_LIST:
	newobj->def.leaflist = 
	    clone_leaflist(srcobj->def.leaflist,
			   (mobj) ? mobj->def.refine : NULL);
	if (!newobj->def.leaflist) {
	    res = ERR_INTERNAL_MEM;
	}
	break;
    case OBJ_TYP_LIST:
	newobj->def.list = 
	    clone_list(mod, 
                       newobj, 
                       srcobj,
		       (mobj) ? mobj->def.refine : NULL, 
		       mobjQ);
	if (!newobj->def.list) {
	    res = ERR_INTERNAL_MEM;
	}
	break;
    case OBJ_TYP_CHOICE:
	newobj->def.choic = 
	    clone_choice(mod, 
                         srcobj->def.choic,
			 (mobj) ? mobj->def.refine : NULL, 
			 newobj, 
                         mobjQ);
	if (!newobj->def.choic) {
	    res = ERR_INTERNAL_MEM;
	}
	break;
    case OBJ_TYP_CASE:
	newobj->def.cas = 
	    clone_case(mod, 
                       srcobj->def.cas,
		       (mobj) ? mobj->def.refine : NULL,
		       newobj, 
                       mobjQ);
	if (!newobj->def.cas) {
	    res = ERR_INTERNAL_MEM;
	}
	break;
    case OBJ_TYP_USES:
	if (mobj) {
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	} else {
	    newobj->def.uses = srcobj->def.uses;
	    newobj->flags |= OBJ_FL_DEFCLONE;
	}
	break;
    case OBJ_TYP_AUGMENT:
	if (mobj) {
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	} else {
	    newobj->def.augment = srcobj->def.augment;
	    newobj->flags |= OBJ_FL_DEFCLONE;
	}
	break;
    case OBJ_TYP_NONE:
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (res != NO_ERR) {
	free_blank_template(newobj);
	return NULL;
    } else {
	return newobj;
    }

}   /* obj_clone_template */


/********************************************************************
* FUNCTION obj_clone_template_case
*
* Clone an obj_template_t but make sure it is wrapped
* in a OBJ_TYP_CASE layer
*
* Copy the pointers from the srcobj into the new obj
*
* If the mobj is non-NULL, then the non-NULL revisable
* fields in the mobj struct will be merged into the new object
*
* INPUTS:
*   mod == module struct that is defining the new cloned data
*          this may be different than the module that will
*          contain the cloned data (except top-level objects)
*   srcobj == obj_template to clone
*             !!! This struct MUST NOT be deleted!!!
*             !!! Unless all of its clones are also deleted !!!
*   mobjQ == Q of obj_refine_t objects to merge (may be NULL)
*           only fields allowed to be revised will be checked
*           even if other fields are set in this struct
*
* RETURNS:
*   pointer to malloced clone obj_template_t
*   NULL if malloc failer error or internal error
*********************************************************************/
obj_template_t *
    obj_clone_template_case (ncx_module_t *mod,
			     obj_template_t *srcobj,
			     dlq_hdr_t *mobjQ)
{
    obj_template_t     *casobj, *newobj;

#ifdef DEBUG
    if (!srcobj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
    if (srcobj->objtype == OBJ_TYP_NONE ||
	srcobj->objtype > OBJ_TYP_AUGMENT) {
        SET_ERROR(ERR_INTERNAL_VAL);
        return NULL;
    }
#endif

    if (srcobj->objtype == OBJ_TYP_CASE) {
	return obj_clone_template(mod, srcobj, mobjQ);
    }

    casobj = new_blank_template();
    if (!casobj) {
	return NULL;
    }

    /* set most of the common fields but leave the mod and parent NULL
     * since the uses or augment calling this fn is going to
     * re-prent the cloned node under a different part of the tree
     */
    casobj->objtype = OBJ_TYP_CASE;
    ncx_set_error(&casobj->tkerr,
                  srcobj->tkerr.mod,
                  srcobj->tkerr.linenum,
                  srcobj->tkerr.linepos);
    casobj->flags = OBJ_FL_CLONE;
    casobj->def.cas = new_case(TRUE);
    if (!casobj->def.cas) {
	free_blank_template(casobj);
	return NULL;
    }
    casobj->def.cas->name = xml_strdup(obj_get_name(srcobj));
    if (!casobj->def.cas->name) {
	obj_free_template(casobj);
	return NULL;
    }
    casobj->def.cas->status = obj_get_status(srcobj);

    newobj = obj_clone_template(mod, srcobj, mobjQ);
    if (!newobj) {
	obj_free_template(casobj);
	return NULL;
    }

    newobj->parent = casobj;
    dlq_enque(newobj, casobj->def.cas->datadefQ);
    return casobj;

}   /* obj_clone_template_case */



/********************************************************************
* FUNCTION obj_new_unique
* 
* Alloc and Init a obj_unique_t struct
*
* RETURNS:
*   pointer to malloced struct or NULL if memory error
*********************************************************************/
obj_unique_t *
    obj_new_unique (void)
{
    obj_unique_t  *un;

    un = m__getObj(obj_unique_t);
    if (!un) {
        return NULL;
    }
    obj_init_unique(un);
    return un;

}  /* obj_new_unique */


/********************************************************************
* FUNCTION obj_init_unique
* 
* Init a obj_unique_t struct
*
* INPUTS:
*   un == obj_unique_t struct to init
*********************************************************************/
void
    obj_init_unique (obj_unique_t *un)
{
#ifdef DEBUG
    if (!un) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif
    memset(un, 0, sizeof(obj_unique_t));
    dlq_createSQue(&un->compQ);

}  /* obj_init_unique */


/********************************************************************
* FUNCTION obj_free_unique
* 
* Free a obj_unique_t struct
*
* INPUTS:
*   un == obj_unique_t struct to free
*********************************************************************/
void
    obj_free_unique (obj_unique_t *un)
{
#ifdef DEBUG
    if (!un) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    obj_clean_unique(un);
    m__free(un);

}  /* obj_free_unique */


/********************************************************************
* FUNCTION obj_clean_unique
* 
* Clean a obj_unique_t struct
*
* INPUTS:
*   un == obj_unique_t struct to clean
*********************************************************************/
void
    obj_clean_unique (obj_unique_t *un)
{
    obj_unique_comp_t *unc;

#ifdef DEBUG
    if (!un) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (un->xpath) {
	m__free(un->xpath);
	un->xpath = NULL;
    }

    while (!dlq_empty(&un->compQ)) {
	unc = (obj_unique_comp_t *)dlq_deque(&un->compQ);
	obj_free_unique_comp(unc);
    }

}  /* obj_clean_unique */


/********************************************************************
* FUNCTION obj_new_unique_comp
* 
* Alloc and Init a obj_unique_comp_t struct
*
* RETURNS:
*   pointer to malloced struct or NULL if memory error
*********************************************************************/
obj_unique_comp_t *
    obj_new_unique_comp (void)
{
    obj_unique_comp_t  *unc;

    unc = m__getObj(obj_unique_comp_t);
    if (!unc) {
        return NULL;
    }
    memset(unc, 0x0, sizeof(obj_unique_comp_t));
    return unc;

}  /* obj_new_unique_comp */


/********************************************************************
* FUNCTION obj_free_unique_comp
* 
* Free a obj_unique_comp_t struct
*
* INPUTS:
*   unc == obj_unique_comp_t struct to free
*********************************************************************/
void
    obj_free_unique_comp (obj_unique_comp_t *unc)
{
#ifdef DEBUG
    if (!unc) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (unc->xpath) {
	m__free(unc->xpath);
    }
    m__free(unc);

}  /* obj_free_unique_comp */


/********************************************************************
* FUNCTION obj_find_unique
* 
* Find a specific unique-stmt
*
* RETURNS:
*   pointer to found entry or NULL if not found
*********************************************************************/
obj_unique_t *
    obj_find_unique (dlq_hdr_t *que,
		     const xmlChar *xpath)
{
    obj_unique_t  *un;

#ifdef DEBUG
    if (!que || !xpath) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (un = (obj_unique_t *)dlq_firstEntry(que);
	 un != NULL;
	 un = (obj_unique_t *)dlq_nextEntry(un)) {
	if (!xml_strcmp(un->xpath, xpath)) {
	    return un;
	}
    }
    return NULL;

}  /* obj_find_unique */


/********************************************************************
* FUNCTION obj_first_unique
* 
* Get the first unique-stmt for a list
*
* INPUTS:
*   listobj == (list) object to check for unique structs
*
* RETURNS:
*   pointer to found entry or NULL if not found
*********************************************************************/
obj_unique_t *
    obj_first_unique (obj_template_t *listobj)
{

#ifdef DEBUG
    if (!listobj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (listobj->objtype != OBJ_TYP_LIST) {
	return NULL;
    }

    return (obj_unique_t *)
	dlq_firstEntry(&listobj->def.list->uniqueQ);

}  /* obj_first_unique */


/********************************************************************
* FUNCTION obj_next_unique
* 
* Get the next unique-stmt for a list
*
* INPUTS:
*  un == current unique node
*
* RETURNS:
*   pointer to found entry or NULL if not found
*********************************************************************/
obj_unique_t *
    obj_next_unique (obj_unique_t *un)
{
#ifdef DEBUG
    if (!un) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (obj_unique_t *)dlq_nextEntry(un);

}  /* obj_next_unique */


/********************************************************************
* FUNCTION obj_first_unique_comp
* 
* Get the first identifier in a unique-stmt for a list
*
* INPUTS:
*   un == unique struct to check
*
* RETURNS:
*   pointer to found entry or NULL if not found
*********************************************************************/
obj_unique_comp_t *
    obj_first_unique_comp (obj_unique_t *un)
{

#ifdef DEBUG
    if (!un) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (obj_unique_comp_t *)dlq_firstEntry(&un->compQ);

}  /* obj_first_unique_comp */


/********************************************************************
* FUNCTION obj_next_unique_comp
* 
* Get the next unique-stmt component for a list
*
* INPUTS:
*  uncomp == current unique component node
*
* RETURNS:
*   pointer to next entry or NULL if none
*********************************************************************/
obj_unique_comp_t *
    obj_next_unique_comp (obj_unique_comp_t *uncomp)
{
#ifdef DEBUG
    if (!uncomp) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (obj_unique_comp_t *)dlq_nextEntry(uncomp);

}  /* obj_next_unique_comp */


/********************************************************************
* FUNCTION obj_new_key
* 
* Alloc and Init a obj_key_t struct
*
* RETURNS:
*   pointer to malloced struct or NULL if memory error
*********************************************************************/
obj_key_t *
    obj_new_key (void)
{
    obj_key_t  *key;

    key = m__getObj(obj_key_t);
    if (!key) {
        return NULL;
    }
    memset(key, 0x0, sizeof(obj_key_t));
    return key;

}  /* obj_new_key */


/********************************************************************
* FUNCTION obj_free_key
* 
* Free a obj_key_t struct
*
* INPUTS:
*   key == obj_key_t struct to free
*********************************************************************/
void
    obj_free_key (obj_key_t *key)
{
#ifdef DEBUG
    if (!key) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    m__free(key);

}  /* obj_free_key */


/********************************************************************
* FUNCTION obj_find_key
* 
* Find a specific key component by key leaf identifier name
* Assumes deep keys are not supported!!!
*
* INPUTS:
*   que == Q of obj_key_t to check
*   keycompname == key component name to find
*
* RETURNS:
*   pointer to found key component or NULL if not found
*********************************************************************/
obj_key_t *
    obj_find_key (dlq_hdr_t *que,
		  const xmlChar *keycompname)
{
    obj_key_t  *key;

#ifdef DEBUG
    if (!que || !keycompname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (key = (obj_key_t *)dlq_firstEntry(que);
	 key != NULL;
	 key = (obj_key_t *)dlq_nextEntry(key)) {
	if (!xml_strcmp(obj_get_name(key->keyobj), keycompname)) {
	    return key;
	}
    }
    return NULL;

}  /* obj_find_key */


/********************************************************************
* FUNCTION obj_find_key2
* 
* Find a specific key component, check for a specific node
* in case deep keys are supported, and to check for duplicates
*
* INPUTS:
*   que == Q of obj_key_t to check
*   keyobj == key component object to find
*
* RETURNS:
*   pointer to found key component or NULL if not found
*********************************************************************/
obj_key_t *
    obj_find_key2 (dlq_hdr_t *que,
		   obj_template_t *keyobj)
{
    obj_key_t  *key;

#ifdef DEBUG
    if (!que || !keyobj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (key = (obj_key_t *)dlq_firstEntry(que);
	 key != NULL;
	 key = (obj_key_t *)dlq_nextEntry(key)) {
	if (keyobj == key->keyobj) {
	    return key;
	}
    }
    return NULL;

}  /* obj_find_key2 */


/********************************************************************
* FUNCTION obj_first_key
* 
* Get the first key record
*
* INPUTS:
*   obj == object to check
*
* RETURNS:
*   pointer to first key component or NULL if not found
*********************************************************************/
obj_key_t *
    obj_first_key (obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
    if (obj->objtype != OBJ_TYP_LIST) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
#endif

    return (obj_key_t *)dlq_firstEntry(&obj->def.list->keyQ);

}  /* obj_first_key */


/********************************************************************
* FUNCTION obj_first_ckey
* 
* Get the first key record: Const version
*
* INPUTS:
*   obj == object to check
*
* RETURNS:
*   pointer to first key component or NULL if not found
*********************************************************************/
const obj_key_t *
    obj_first_ckey (const obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
    if (obj->objtype != OBJ_TYP_LIST) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
#endif

    return (const obj_key_t *)dlq_firstEntry(&obj->def.list->keyQ);

}  /* obj_first_ckey */


/********************************************************************
* FUNCTION obj_next_key
* 
* Get the next key record
*
* INPUTS:
*   objkey == current key record
*
* RETURNS:
*   pointer to next key component or NULL if not found
*********************************************************************/
obj_key_t *
    obj_next_key (obj_key_t *objkey)
{
#ifdef DEBUG
    if (!objkey) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (obj_key_t *)dlq_nextEntry(objkey);

}  /* obj_next_key */


/********************************************************************
* FUNCTION obj_next_ckey
* 
* Get the next key record: Const version
*
* INPUTS:
*   objkey == current key record
*
* RETURNS:
*   pointer to next key component or NULL if not found
*********************************************************************/
const obj_key_t *
    obj_next_ckey (const obj_key_t *objkey)
{
#ifdef DEBUG
    if (!objkey) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (const obj_key_t *)dlq_nextEntry(objkey);

}  /* obj_next_ckey */


/********************************************************************
* FUNCTION obj_key_count
* 
* Get the number of keys for this object
*
* INPUTS:
*   obj == object to check
*
* RETURNS:
*   number of keys in the obj_key_t Q
*********************************************************************/
uint32
    obj_key_count (const obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return 0;
    }
#endif

    if (obj->objtype != OBJ_TYP_LIST) {
	return 0;
    }

    return dlq_count(&obj->def.list->keyQ);

}  /* obj_key_count */


/********************************************************************
* FUNCTION obj_any_rpcs
* 
* Check if there are any RPC methods in the datadefQ
*
* INPUTS:
*   que == Q of obj_template_t to check
*
* RETURNS:
*   TRUE if any OBJ_TYP_RPC found, FALSE if not
*********************************************************************/
boolean
    obj_any_rpcs (const dlq_hdr_t *datadefQ)
{
    const obj_template_t  *obj;

#ifdef DEBUG
    if (!datadefQ) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    for (obj = (const obj_template_t *)dlq_firstEntry(datadefQ);
	 obj != NULL;
	 obj = (const obj_template_t *)dlq_nextEntry(obj)) {
	if (obj->objtype == OBJ_TYP_RPC) {
	    return TRUE;
	}
    }
    return FALSE;

}  /* obj_any_rpcs */


/********************************************************************
* FUNCTION obj_any_notifs
* 
* Check if there are any notifications in the datadefQ
*
* INPUTS:
*   que == Q of obj_template_t to check
*
* RETURNS:
*   TRUE if any OBJ_TYP_NOTIF found, FALSE if not
*********************************************************************/
boolean
    obj_any_notifs (const dlq_hdr_t *datadefQ)
{
    const obj_template_t  *obj;

#ifdef DEBUG
    if (!datadefQ) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    for (obj = (const obj_template_t *)dlq_firstEntry(datadefQ);
	 obj != NULL;
	 obj = (const obj_template_t *)dlq_nextEntry(obj)) {
	if (obj->objtype == OBJ_TYP_NOTIF) {
	    return TRUE;
	}
    }
    return FALSE;

}  /* obj_any_notifs */


/********************************************************************
* FUNCTION obj_new_deviate
* 
* Malloc and initialize the fields in a an object deviate statement
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
obj_deviate_t * 
    obj_new_deviate (void)
{
    obj_deviate_t  *deviate;

    deviate = m__getObj(obj_deviate_t);
    if (!deviate) {
	return NULL;
    }

    memset(deviate, 0x0, sizeof(obj_deviate_t));

    dlq_createSQue(&deviate->mustQ);
    dlq_createSQue(&deviate->uniqueQ);
    dlq_createSQue(&deviate->appinfoQ);

    return deviate;

} /* obj_new_deviate */


/********************************************************************
* FUNCTION obj_free_deviate
* 
* Clean and free an object deviate statement
*
* INPUTS:
*   deviate == pointer to the struct to clean and free
*********************************************************************/
void
    obj_free_deviate (obj_deviate_t *deviate)
{
    obj_unique_t   *uni;

#ifdef DEBUG
    if (!deviate) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (deviate->typdef) {
	typ_free_typdef(deviate->typdef);
    }
    if (deviate->units) {
	m__free(deviate->units);
    }
    if (deviate->defval) {
	m__free(deviate->defval);
    }

    clean_mustQ(&deviate->mustQ);

    while (!dlq_empty(&deviate->uniqueQ)) {
	uni = (obj_unique_t *)dlq_deque(&deviate->uniqueQ);
	obj_free_unique(uni);
    }

    ncx_clean_appinfoQ(&deviate->appinfoQ);

    m__free(deviate);

} /* obj_free_deviate */


/********************************************************************
* FUNCTION obj_get_deviate_arg
* 
* Get the deviate-arg string from its enumeration
*
* INPUTS:
*   devarg == enumeration to convert
* RETURNS:
*   const string version of the enum
*********************************************************************/
const xmlChar *
    obj_get_deviate_arg (obj_deviate_arg_t devarg)
{
    switch (devarg) {
    case OBJ_DARG_NONE:
        return NCX_EL_NONE;
    case OBJ_DARG_ADD:
        return YANG_K_ADD;
    case OBJ_DARG_DELETE:
        return YANG_K_DELETE;
    case OBJ_DARG_REPLACE:
        return YANG_K_REPLACE;
    case OBJ_DARG_NOT_SUPPORTED:
        return YANG_K_NOT_SUPPORTED;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return (const xmlChar *)"--";
    }

} /* obj_get_deviate_arg */


/********************************************************************
* FUNCTION obj_new_deviation
* 
* Malloc and initialize the fields in a an object deviation statement
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
obj_deviation_t * 
    obj_new_deviation (void)
{
    obj_deviation_t  *deviation;

    deviation = m__getObj(obj_deviation_t);
    if (!deviation) {
	return NULL;
    }

    memset(deviation, 0x0, sizeof(obj_deviation_t));

    dlq_createSQue(&deviation->deviateQ);
    dlq_createSQue(&deviation->appinfoQ);

    return deviation;

} /* obj_new_deviation */


/********************************************************************
* FUNCTION obj_free_deviation
* 
* Clean and free an object deviation statement
*
* INPUTS:
*   deviation == pointer to the struct to clean and free
*********************************************************************/
void
    obj_free_deviation (obj_deviation_t *deviation)
{
    obj_deviate_t   *deviate;

#ifdef DEBUG
    if (!deviation) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (deviation->target) {
	m__free(deviation->target);
    }
    if (deviation->targmodname) {
	m__free(deviation->targmodname);
    }
    if (deviation->descr) {
	m__free(deviation->descr);
    }
    if (deviation->ref) {
	m__free(deviation->ref);
    }
    if (deviation->devmodname) {
	m__free(deviation->devmodname);
    }

    while (!dlq_empty(&deviation->deviateQ)) {
	deviate = (obj_deviate_t *)
	    dlq_deque(&deviation->deviateQ);
	obj_free_deviate(deviate);
    }

    ncx_clean_appinfoQ(&deviation->appinfoQ);

    m__free(deviation);

} /* obj_free_deviation */


/********************************************************************
* FUNCTION obj_clean_deviationQ
* 
* Clean and free an Q of object deviation statements
*
* INPUTS:
*   deviationQ == pointer to Q of the structs to clean and free
*********************************************************************/
void
    obj_clean_deviationQ (dlq_hdr_t *deviationQ)
{
    obj_deviation_t   *deviation;

#ifdef DEBUG
    if (!deviationQ) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    while (!dlq_empty(deviationQ)) {
	deviation = (obj_deviation_t *)dlq_deque(deviationQ);
	obj_free_deviation(deviation);
    }

} /* obj_clean_deviationQ */


/********************************************************************
* FUNCTION obj_gen_object_id
* 
* Malloc and Generate the object ID for an object node
* 
* INPUTS:
*   obj == node to generate the instance ID for
*   buff == pointer to address of buffer to use
*
* OUTPUTS
*   *buff == malloced buffer with the instance ID
*
* RETURNS:
*   status
*********************************************************************/
status_t
    obj_gen_object_id (const obj_template_t *obj,
		       xmlChar  **buff)
{
    uint32    len;
    status_t  res;

#ifdef DEBUG 
    if (!obj || !buff) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    *buff = NULL;

    /* figure out the length of the object ID */
    res = get_object_string(obj, NULL, 0, TRUE, NULL, &len);
    if (res != NO_ERR) {
	return res;
    }

    /* get a buffer to fit the instance ID string */
    *buff = (xmlChar *)m__getMem(len+1);
    if (!*buff) {
	return ERR_INTERNAL_MEM;
    }

    /* get the object ID for real this time */
    res = get_object_string(obj, *buff, len+1, TRUE, NULL, &len);
    if (res != NO_ERR) {
	m__free(*buff);
	*buff = NULL;
	return SET_ERROR(res);
    }

    return NO_ERR;

}  /* obj_gen_object_id */


/********************************************************************
* FUNCTION obj_gen_object_id_code
* 
* Malloc and Generate the object ID for an object node
* for C code usage
*
* INPUTS:
*   mod == current module in progress
*   obj == node to generate the instance ID for
*   buff == pointer to address of buffer to use
*
* OUTPUTS
*   *buff == malloced buffer with the instance ID
*
* RETURNS:
*   status
*********************************************************************/
status_t
    obj_gen_object_id_code (ncx_module_t *mod,
                            const obj_template_t *obj,
                            xmlChar  **buff)
{
    uint32    len;
    status_t  res;

#ifdef DEBUG 
    if (!mod || !obj || !buff) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    *buff = NULL;

    /* figure out the length of the object ID */
    res = get_object_string(obj, NULL, 0, TRUE, mod, &len);
    if (res != NO_ERR) {
	return res;
    }

    /* get a buffer to fit the instance ID string */
    *buff = (xmlChar *)m__getMem(len+1);
    if (!*buff) {
	return ERR_INTERNAL_MEM;
    }

    /* get the object ID for real this time */
    res = get_object_string(obj, *buff, len+1, TRUE, mod, &len);
    if (res != NO_ERR) {
	m__free(*buff);
	*buff = NULL;
	return SET_ERROR(res);
    }

    return NO_ERR;

}  /* obj_gen_object_id_code */


/********************************************************************
* FUNCTION obj_copy_object_id
* 
* Generate the object ID for an object node and copy to the buffer
* 
* INPUTS:
*   obj == node to generate the instance ID for
*   buff == buffer to use
*   bufflen == size of buff
*   reallen == address of return length of actual identifier 
*               (may be NULL)
*
* OUTPUTS
*   buff == filled in with the object ID
*  if reallen not NULL:
*     *reallen == length of identifier, even if error occurred
*  
* RETURNS:
*   status
*********************************************************************/
status_t
    obj_copy_object_id (const obj_template_t *obj,
			xmlChar  *buff,
			uint32 bufflen,
			uint32 *reallen)
{
#ifdef DEBUG 
    if (!obj || !buff) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    return get_object_string(obj, 
                             buff, 
                             bufflen, 
                             TRUE, 
                             NULL, 
                             reallen);

}  /* obj_copy_object_id */


/********************************************************************
* FUNCTION obj_gen_aughook_id
* 
* Malloc and Generate the augment hook element name for
* the specified object. This will be a child node of the
* specified object.
* 
* INPUTS:
*   obj == node to generate the augment hook ID for
*   buff == pointer to address of buffer to use
*
* OUTPUTS
*   *buff == malloced buffer with the instance ID
*
* RETURNS:
*   status
*********************************************************************/
status_t
    obj_gen_aughook_id (const obj_template_t *obj,
			xmlChar  **buff)
{
    xmlChar  *p;
    uint32    len, extra;
    status_t  res;

#ifdef DEBUG 
    if (!obj || !buff) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    *buff = NULL;

    /* figure out the length of the aughook ID */
    res = get_object_string(obj, NULL, 0, FALSE, NULL, &len);
    if (res != NO_ERR) {
	return res;
    }

    /* get the length for the aughook prefix and suffix */
    extra = (xml_strlen(NCX_AUGHOOK_START) + xml_strlen(NCX_AUGHOOK_END));

    /* get a buffer to fit the instance ID string */
    *buff = (xmlChar *)m__getMem(len+extra+1);
    if (!*buff) {
	return ERR_INTERNAL_MEM;
    }

    /* put prefix in buffer */
    p = *buff;
    p += xml_strcpy(p, NCX_AUGHOOK_START);
    
    /* add the aughook ID to the buffer */
    res = get_object_string(obj, p, len+1, FALSE, NULL, &len);
    if (res != NO_ERR) {
	m__free(*buff);
	*buff = NULL;
	return SET_ERROR(res);
    }

    /* add suffix to the buffer */
    p += len;
    xml_strcpy(p, NCX_AUGHOOK_END);

    return NO_ERR;

}  /* obj_gen_aughook_id */


/********************************************************************
* FUNCTION obj_get_name
* 
* Get the name field for this obj
*
* INPUTS:
*   obj == the specific object to check
*
* RETURNS:
*   pointer to the name field, NULL if some error or unnamed
*********************************************************************/
const xmlChar * 
    obj_get_name (const obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return (const xmlChar *)"<none>";
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	return obj->def.container->name;
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_LEAF:
	return obj->def.leaf->name;
    case OBJ_TYP_LEAF_LIST:
	return obj->def.leaflist->name;
    case OBJ_TYP_LIST:
	return obj->def.list->name;
    case OBJ_TYP_CHOICE:
	return obj->def.choic->name;
    case OBJ_TYP_CASE:
	return obj->def.cas->name;
    case OBJ_TYP_USES:
	return YANG_K_USES;
    case OBJ_TYP_AUGMENT:
	return YANG_K_AUGMENT;
    case OBJ_TYP_REFINE:
	return YANG_K_REFINE;
    case OBJ_TYP_RPC:
	return obj->def.rpc->name;
    case OBJ_TYP_RPCIO:
	return obj->def.rpcio->name;
    case OBJ_TYP_NOTIF:
	return obj->def.notif->name;
    case OBJ_TYP_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NCX_EL_NONE;
    }
    /*NOTREACHED*/

}  /* obj_get_name */


/********************************************************************
* FUNCTION obj_has_name
* 
* Check if the specified object type has a name
*
* INPUTS:
*   obj == the specific object to check
*
* RETURNS:
*   TRUE if obj has a name
*   FALSE otherwise
*********************************************************************/
boolean
    obj_has_name (const obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
    case OBJ_TYP_LIST:
    case OBJ_TYP_CHOICE:
    case OBJ_TYP_CASE:
	return TRUE;
    case OBJ_TYP_USES:
    case OBJ_TYP_AUGMENT:
    case OBJ_TYP_REFINE:
	return FALSE;
    case OBJ_TYP_RPC:
    case OBJ_TYP_RPCIO:
    case OBJ_TYP_NOTIF:
	return TRUE;
    case OBJ_TYP_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }
    /*NOTREACHED*/

}  /* obj_has_name */


/********************************************************************
* FUNCTION obj_has_text_content
* 
* Check if the specified object type has a text content
* for XPath purposes
*
* INPUTS:
*   obj == the specific object to check
*
* RETURNS:
*   TRUE if obj has text content
*   FALSE otherwise
*********************************************************************/
boolean
    obj_has_text_content (const obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
	return TRUE;
    default:
	return FALSE;
    }
    /*NOTREACHED*/

}  /* obj_has_text_content */


/********************************************************************
* FUNCTION obj_get_status
* 
* Get the status field for this obj
*
* INPUTS:
*   obj == the specific object to check
*
* RETURNS:
*   YANG status clause for this object
*********************************************************************/
ncx_status_t
    obj_get_status (const obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NCX_STATUS_NONE;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	return obj->def.container->status;
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_LEAF:
	return obj->def.leaf->status;
    case OBJ_TYP_LEAF_LIST:
	return obj->def.leaflist->status;
    case OBJ_TYP_LIST:
	return obj->def.list->status;
    case OBJ_TYP_CHOICE:
	return obj->def.choic->status;
    case OBJ_TYP_CASE:
    case OBJ_TYP_REFINE:
	return (obj->parent) ?
	    obj_get_status(obj->parent) : NCX_STATUS_CURRENT;
    case OBJ_TYP_USES:
	return obj->def.uses->status;
    case OBJ_TYP_AUGMENT:
	return obj->def.augment->status;
    case OBJ_TYP_RPC:
	return obj->def.rpc->status;
    case OBJ_TYP_RPCIO:
	return (obj->parent) ?
	    obj_get_status(obj->parent) : NCX_STATUS_CURRENT;
    case OBJ_TYP_NOTIF:
	return obj->def.notif->status;
    case OBJ_TYP_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NCX_STATUS_NONE;
    }
    /*NOTREACHED*/

}  /* obj_get_status */


/********************************************************************
* FUNCTION obj_get_description
* 
* Get the description field for this obj
*
* INPUTS:
*   obj == the specific object to check
*
* RETURNS:
*   YANG status clause for this object
*********************************************************************/
const xmlChar *
    obj_get_description (const obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	return obj->def.container->descr;
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_LEAF:
	return obj->def.leaf->descr;
    case OBJ_TYP_LEAF_LIST:
	return obj->def.leaflist->descr;
    case OBJ_TYP_LIST:
	return obj->def.list->descr;
    case OBJ_TYP_CHOICE:
	return obj->def.choic->descr;
    case OBJ_TYP_CASE:
	return obj->def.cas->descr;
    case OBJ_TYP_USES:
	return obj->def.uses->descr;
    case OBJ_TYP_REFINE:
	return obj->def.refine->descr;
    case OBJ_TYP_AUGMENT:
	return obj->def.augment->descr;
    case OBJ_TYP_RPC:
	return obj->def.rpc->descr;
    case OBJ_TYP_RPCIO:
	return NULL;
    case OBJ_TYP_NOTIF:
	return obj->def.notif->descr;
    case OBJ_TYP_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
    /*NOTREACHED*/

}  /* obj_get_description */


/********************************************************************
* FUNCTION obj_get_reference
* 
* Get the reference field for this obj
*
* INPUTS:
*   obj == the specific object to check
*
* RETURNS:
*   YANG status clause for this object
*********************************************************************/
const xmlChar *
    obj_get_reference (const obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	return obj->def.container->ref;
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_LEAF:
	return obj->def.leaf->ref;
    case OBJ_TYP_LEAF_LIST:
	return obj->def.leaflist->ref;
    case OBJ_TYP_LIST:
	return obj->def.list->ref;
    case OBJ_TYP_CHOICE:
	return obj->def.choic->ref;
    case OBJ_TYP_CASE:
	return obj->def.cas->ref;
    case OBJ_TYP_USES:
	return obj->def.uses->ref;
    case OBJ_TYP_REFINE:
	return obj->def.refine->ref;
    case OBJ_TYP_AUGMENT:
	return obj->def.augment->ref;
    case OBJ_TYP_RPC:
	return obj->def.rpc->ref;
    case OBJ_TYP_RPCIO:
	return NULL;
    case OBJ_TYP_NOTIF:
	return obj->def.notif->ref;
    case OBJ_TYP_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
    /*NOTREACHED*/

}  /* obj_get_reference */


/********************************************************************
* FUNCTION obj_get_config_flag
*
* Get the config flag for an obj_template_t 
* Return the explicit value or the inherited value
* Also return if the config-stmt is really set or not
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if config set to TRUE
*   FALSE if config set to FALSE
*   
*********************************************************************/
boolean
    obj_get_config_flag (const obj_template_t *obj)
{
    boolean setflag, retval;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    retval = get_config_flag(obj, &setflag);
    return retval;

}   /* obj_get_config_flag */


/********************************************************************
* FUNCTION obj_get_config_flag2
*
* Get the config flag for an obj_template_t 
* Return the explicit value or the inherited value
* Also return if the config-stmt is really set or not
*
* INPUTS:
*   obj == obj_template to check
*   setflag == address of return config-stmt set flag
*
* OUTPUTS:
*   *setflag == TRUE if the config-stmt is set in this
*               node, or if it is a top-level object
*            == FALSE if the config-stmt is inherited from its parent
*
* RETURNS:
*   TRUE if config set to TRUE
*   FALSE if config set to FALSE
*   
*********************************************************************/
boolean
    obj_get_config_flag2 (const obj_template_t *obj,
			 boolean *setflag)
{
#ifdef DEBUG
    if (!obj || !setflag) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif
    return get_config_flag(obj, setflag);

}   /* obj_get_config_flag2 */


/********************************************************************
* FUNCTION obj_get_max_access
*
* Get the NCX max-access enum for an obj_template_t 
* Return the explicit value or the inherited value
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   ncx_access_t enumeration
*********************************************************************/
ncx_access_t
    obj_get_max_access (const obj_template_t *obj)
{
    boolean      setflag;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NCX_ACCESS_NONE;
    }
#endif

    if (!get_config_flag(obj, &setflag)) {
	return NCX_ACCESS_RO;
    } else {
	return NCX_ACCESS_RC;
    }

    /*** !!! no support for read-write at this time !!! ***/

}   /* obj_get_max_access */


/********************************************************************
* FUNCTION obj_get_appinfoQ
* 
* Get the appinfoQ for this obj
*
* INPUTS:
*   obj == the specific object to check
*
* RETURNS:
*   YANG status clause for this object
*********************************************************************/
dlq_hdr_t *
    obj_get_appinfoQ (obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return &obj->appinfoQ;

}  /* obj_get_appinfoQ */


/********************************************************************
* FUNCTION obj_get_appinfoQ2
* 
* Get the appinfoQ for this obj (not const)
*
* INPUTS:
*   obj == the specific object to check
*
* RETURNS:
*   YANG status clause for this object
*********************************************************************/
dlq_hdr_t *
    obj_get_appinfoQ2 (obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return &obj->appinfoQ;

}  /* obj_get_appinfoQ2 */


/********************************************************************
* FUNCTION obj_get_mustQ
* 
* Get the mustQ for this obj
*
* INPUTS:
*   obj == the specific object to check
*
* RETURNS:
*   YANG status clause for this object
*********************************************************************/
dlq_hdr_t *
    obj_get_mustQ (const obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	return &obj->def.container->mustQ;
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_LEAF:
	return &obj->def.leaf->mustQ;
    case OBJ_TYP_LEAF_LIST:
	return &obj->def.leaflist->mustQ;
    case OBJ_TYP_LIST:
	return &obj->def.list->mustQ;
    case OBJ_TYP_REFINE:
	return &obj->def.refine->mustQ;
    default:
	return NULL;
    }
    /*NOTREACHED*/

}  /* obj_get_mustQ */


/********************************************************************
* FUNCTION obj_get_typestr
*
* Get the name of the object type
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   name string for this object type
*********************************************************************/
const xmlChar *
    obj_get_typestr (const obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NCX_EL_NONE;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	return YANG_K_CONTAINER;
    case OBJ_TYP_ANYXML:
	return YANG_K_ANYXML;
    case OBJ_TYP_LEAF:
	return YANG_K_LEAF;
    case OBJ_TYP_LEAF_LIST:
	return YANG_K_LEAF_LIST;
    case OBJ_TYP_LIST:
	return YANG_K_LIST;
    case OBJ_TYP_CHOICE:
	return YANG_K_CHOICE;
    case OBJ_TYP_CASE:
	return YANG_K_CASE;
    case OBJ_TYP_USES:
	return YANG_K_USES;
    case OBJ_TYP_REFINE:
	return YANG_K_REFINE;
    case OBJ_TYP_AUGMENT:
	return YANG_K_AUGMENT;
    case OBJ_TYP_RPC:
	return YANG_K_RPC;
    case OBJ_TYP_RPCIO:
	return YANG_K_CONTAINER;
    case OBJ_TYP_NOTIF:
	return YANG_K_NOTIFICATION;
    case OBJ_TYP_NONE:
	return NCX_EL_NONE;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NCX_EL_NONE;
    }
    /*NOTREACHED*/

}   /* obj_get_typestr */


/********************************************************************
* FUNCTION obj_get_datadefQ
*
* Get the datadefQ (or caseQ) if this object has one
*
* INPUTS:
*   obj == object to check
*
* RETURNS:
*    pointer to Q of obj_template, or NULL if none
*********************************************************************/
dlq_hdr_t *
    obj_get_datadefQ (obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	return obj->def.container->datadefQ;
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
    case OBJ_TYP_REFINE:
	return NULL;
    case OBJ_TYP_LIST:
	return obj->def.list->datadefQ;
    case OBJ_TYP_CHOICE:
	return obj->def.choic->caseQ;
    case OBJ_TYP_CASE:
	return obj->def.cas->datadefQ;
    case OBJ_TYP_USES:
	return obj->def.uses->datadefQ;
    case OBJ_TYP_AUGMENT:
	return &obj->def.augment->datadefQ;
    case OBJ_TYP_RPC:
	return &obj->def.rpc->datadefQ;
    case OBJ_TYP_RPCIO:
	return &obj->def.rpcio->datadefQ;
    case OBJ_TYP_NOTIF:
	return &obj->def.notif->datadefQ;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
    /*NOTREACHED*/

}   /* obj_get_datadefQ */


/********************************************************************
* FUNCTION obj_get_cdatadefQ
*
* Get a const pointer to the datadefQ (or caseQ) if this object has one
*
* INPUTS:
*   obj == object to check
*
* RETURNS:
*    pointer to Q of obj_template, or NULL if none
*********************************************************************/
const dlq_hdr_t *
    obj_get_cdatadefQ (const obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	return obj->def.container->datadefQ;
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
    case OBJ_TYP_REFINE:
	return NULL;
    case OBJ_TYP_LIST:
	return obj->def.list->datadefQ;
    case OBJ_TYP_CHOICE:
	return obj->def.choic->caseQ;
    case OBJ_TYP_CASE:
	return obj->def.cas->datadefQ;
    case OBJ_TYP_USES:
	return obj->def.uses->datadefQ;
    case OBJ_TYP_AUGMENT:
	return &obj->def.augment->datadefQ;
    case OBJ_TYP_RPC:
	return &obj->def.rpc->datadefQ;
    case OBJ_TYP_RPCIO:
	return &obj->def.rpcio->datadefQ;
    case OBJ_TYP_NOTIF:
	return &obj->def.notif->datadefQ;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
    /*NOTREACHED*/

}   /* obj_get_cdatadefQ */


/********************************************************************
* FUNCTION obj_get_default
* 
* Get the default value for the specified object
* Only OBJ_TYP_LEAF objtype is supported
* If the leaf has nodefault, then the type is checked
* Choice defaults are ignored.
*
* INPUTS:
*   obj == object to check
*  
* RETURNS:
*   pointer to default value string or NULL if none
*********************************************************************/
const xmlChar *
    obj_get_default (const obj_template_t *obj)
{
#ifdef DEBUG 
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (obj->objtype != OBJ_TYP_LEAF) {
	return NULL;
    }
    if (obj->def.leaf->defval) {
	return obj->def.leaf->defval;
    }
    return typ_get_default(obj->def.leaf->typdef);

}  /* obj_get_default */


/********************************************************************
* FUNCTION obj_get_default_case
* 
* Get the default case for the specified OBJ_TYP_CHOICE object
*
* INPUTS:
*   obj == object to check
*  
* RETURNS:
*   pointer to default case object template OBJ_TYP_CASE
*********************************************************************/
obj_template_t *
    obj_get_default_case (obj_template_t *obj)
{
#ifdef DEBUG 
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
    if (obj->objtype != OBJ_TYP_CHOICE) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
#endif

    if (obj->def.choic->defval) {
	return obj_find_child(obj, 
                              obj_get_mod_name(obj),
			      obj->def.choic->defval);
    }
    return NULL;

}  /* obj_get_default_case */


/********************************************************************
* FUNCTION obj_get_level
* 
* Get the nest level for the specified object
* Top-level is '1'
* Does not count groupings as a level
*
* INPUTS:
*   obj == object to check
*  
* RETURNS:
*   level that this object is located, by checking the parent chain
*********************************************************************/
uint32
    obj_get_level (const obj_template_t *obj)
{
    const obj_template_t  *parent;
    uint32           level;

#ifdef DEBUG 
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return 0;
    }
#endif

    level = 1;
    parent = obj->parent;
    while (parent && !obj_is_root(parent)) {
	level++;
	parent = parent->parent;
    }
    return level;

}  /* obj_get_level */


/********************************************************************
* FUNCTION obj_has_typedefs
* 
* Check if the object has any nested typedefs in it
* This will obly be called if the object is defined in a
* grouping.
*
* INPUTS:
*   obj == object to check
*  
* RETURNS:
*   TRUE if any nested typedefs, FALSE otherwise
*********************************************************************/
boolean
    obj_has_typedefs (const obj_template_t *obj)
{
    const obj_template_t *chobj;
    const grp_template_t *grp;
    const dlq_hdr_t      *typedefQ, *groupingQ, *datadefQ;

#ifdef DEBUG 
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	typedefQ = obj->def.container->typedefQ;
	groupingQ = obj->def.container->groupingQ;
	datadefQ = obj->def.container->datadefQ;
	break;
    case OBJ_TYP_LIST:
	typedefQ = obj->def.list->typedefQ;
	groupingQ = obj->def.list->groupingQ;
	datadefQ = obj->def.list->datadefQ;
	break;
    case OBJ_TYP_RPC:
	typedefQ = &obj->def.rpc->typedefQ;
	groupingQ = &obj->def.rpc->groupingQ;
	datadefQ = &obj->def.rpc->datadefQ;
	break;
    case OBJ_TYP_RPCIO:
	typedefQ = &obj->def.rpcio->typedefQ;
	groupingQ = &obj->def.rpcio->groupingQ;
	datadefQ = &obj->def.rpcio->datadefQ;
	break;
    case OBJ_TYP_NOTIF:
	typedefQ = &obj->def.notif->typedefQ;
	groupingQ = &obj->def.notif->groupingQ;
	datadefQ = &obj->def.notif->datadefQ;
	break;
    default:
	return FALSE;
    }


    if (!dlq_empty(typedefQ)) {
	return TRUE;
    }
	
    for (grp = (const grp_template_t *)dlq_firstEntry(groupingQ);
	 grp != NULL;
	 grp = (const grp_template_t *)dlq_nextEntry(grp)) {
	if (grp_has_typedefs(grp)) {
	    return TRUE;
	}
    }

    for (chobj = (const obj_template_t *)dlq_firstEntry(datadefQ);
	 chobj != NULL;
	 chobj = (const obj_template_t *)dlq_nextEntry(chobj)) {
	if (obj_has_typedefs(chobj)) {
	    return TRUE;
	}
    }

    return FALSE;

}  /* obj_has_typedefs */


/********************************************************************
* FUNCTION obj_get_typdef
* 
* Get the typdef for the leaf or leaf-list
*
* INPUTS:
*    obj  == object to check
*
* RETURNS:
*    pointer to the typdef or NULL if this object type does not
*    have a typdef
*********************************************************************/
typ_def_t *
    obj_get_typdef (obj_template_t  *obj)
{
    if (obj->objtype == OBJ_TYP_LEAF ||
        obj->objtype == OBJ_TYP_ANYXML) {
	return obj->def.leaf->typdef;
    } else if (obj->objtype == OBJ_TYP_LEAF_LIST) {
	return obj->def.leaflist->typdef;
    } else {
	return NULL;
    }
    /*NOTREACHED*/

}  /* obj_get_typdef */


/********************************************************************
* FUNCTION obj_get_ctypdef
* 
* Get the typdef for the leaf or leaf-list : Const version
*
* INPUTS:
*    obj  == object to check
*
* RETURNS:
*    pointer to the typdef or NULL if this object type does not
*    have a typdef
*********************************************************************/
const typ_def_t *
    obj_get_ctypdef (const obj_template_t  *obj)
{
    if (obj->objtype == OBJ_TYP_LEAF ||
        obj->objtype == OBJ_TYP_ANYXML) {
	return obj->def.leaf->typdef;
    } else if (obj->objtype == OBJ_TYP_LEAF_LIST) {
	return obj->def.leaflist->typdef;
    } else {
	return NULL;
    }
    /*NOTREACHED*/

}  /* obj_get_ctypdef */


/********************************************************************
* FUNCTION obj_get_basetype
* 
* Get the NCX base type enum for the object type
*
* INPUTS:
*    obj  == object to check
*
* RETURNS:
*    base type enumeration
*********************************************************************/
ncx_btype_t
    obj_get_basetype (const obj_template_t  *obj)
{
    switch (obj->objtype) {
    case OBJ_TYP_LEAF:
	return typ_get_basetype(obj->def.leaf->typdef);
    case OBJ_TYP_LEAF_LIST:
	return typ_get_basetype(obj->def.leaflist->typdef);
    case OBJ_TYP_CONTAINER:
	return NCX_BT_CONTAINER;
    case OBJ_TYP_LIST:
	return NCX_BT_LIST;
    case OBJ_TYP_CHOICE:
	return NCX_BT_CHOICE;
    case OBJ_TYP_CASE:
	return NCX_BT_CASE;
    case OBJ_TYP_RPC:
	return NCX_BT_CONTAINER;
    case OBJ_TYP_RPCIO:
	return NCX_BT_CONTAINER;
    case OBJ_TYP_NOTIF:
	return NCX_BT_CONTAINER;
    case OBJ_TYP_ANYXML:
	return NCX_BT_ANY;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NCX_BT_NONE;
    }
    /*NOTREACHED*/

}  /* obj_get_basetype */


/********************************************************************
* FUNCTION obj_get_mod_prefix
* 
* Get the module prefix for this object
*
* INPUTS:
*    obj  == object to check
*
* RETURNS:
*    const pointer to mod prefix
*********************************************************************/
const xmlChar *
    obj_get_mod_prefix (const obj_template_t  *obj)
{

    return ncx_get_mod_prefix(obj->tkerr.mod);

}  /* obj_get_mod_prefix */


/********************************************************************
* FUNCTION obj_get_mod_xmlprefix
* 
* Get the module prefix for this object
*
* INPUTS:
*    obj  == object to check
*
* RETURNS:
*    const pointer to mod XML prefix
*********************************************************************/
const xmlChar *
    obj_get_mod_xmlprefix (const obj_template_t  *obj)
{

    return ncx_get_mod_xmlprefix(obj->tkerr.mod);

}  /* obj_get_mod_xmlprefix */


/********************************************************************
* FUNCTION obj_get_mod_name
* 
* Get the module name for this object
*
* INPUTS:
*    obj  == object to check
*
* RETURNS:
*    const pointer to mod prefix
*********************************************************************/
const xmlChar *
    obj_get_mod_name (const obj_template_t  *obj)
{
#ifdef DEBUG
    if (!obj || !obj->tkerr.mod) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (obj->tkerr.mod->ismod) {
	return obj->tkerr.mod->name;
    } else {
	return obj->tkerr.mod->belongs;
    }

}  /* obj_get_mod_name */


/********************************************************************
* FUNCTION obj_get_mod_version
* 
* Get the module version for this object
*
* INPUTS:
*    obj  == object to check
*
* RETURNS:
*    const pointer to mod version or NULL if none
*********************************************************************/
const xmlChar *
    obj_get_mod_version (const obj_template_t  *obj)
{
#ifdef DEBUG
    if (!obj || !obj->tkerr.mod) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return obj->tkerr.mod->version;

}  /* obj_get_mod_version */


/********************************************************************
* FUNCTION obj_get_type_name
* 
* Get the typename for an object
*
* INPUTS:
*    obj  == object to check
*
* RETURNS:
*    const pointer to type name string
*********************************************************************/
const xmlChar *
    obj_get_type_name (const obj_template_t  *obj)
{
    const typ_def_t *typdef;



#ifdef DEBUG
    if (!obj || !obj->tkerr.mod) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    typdef = obj_get_ctypdef(obj);
    if (typdef) {
	if (typdef->typename) {
	    return typdef->typename;
	} else {
	    return (const xmlChar *)
		tk_get_btype_sym(obj_get_basetype(obj));
	}
    } else {
	return obj_get_typestr(obj);
    }

}  /* obj_get_type_name */


/********************************************************************
* FUNCTION obj_get_nsid
* 
* Get the namespace ID for this object
*
* INPUTS:
*    obj  == object to check
*
* RETURNS:
*    namespace ID
*********************************************************************/
xmlns_id_t
    obj_get_nsid (const obj_template_t  *obj)
{
#ifdef DEBUG
    if (!obj || !obj->tkerr.mod) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return 0;
    }
#endif

    return obj->nsid;

}  /* obj_get_nsid */


/********************************************************************
* FUNCTION obj_get_iqualval
* 
* Get the instance qualifier for this object
*
* INPUTS:
*    obj  == object to check
*
* RETURNS:
*    instance qualifier enumeration
*********************************************************************/
ncx_iqual_t
    obj_get_iqualval (obj_template_t  *obj)
{
    boolean      required;

#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NCX_IQUAL_NONE;
    }
#endif

    required = obj_is_mandatory(obj);
    return obj_get_iqualval_ex(obj, required);
    
}  /* obj_get_iqualval */


/********************************************************************
* FUNCTION obj_get_iqualval_ex
* 
* Get the instance qualifier for this object
*
* INPUTS:
*    obj  == object to check
*    mand == value to use for 'is_mandatory()' logic
*
* RETURNS:
*    instance qualifier enumeration
*********************************************************************/
ncx_iqual_t
    obj_get_iqualval_ex (obj_template_t  *obj,
			 boolean required)
{
    ncx_iqual_t  ret;

#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NCX_IQUAL_NONE;
    }
#endif

    ret = NCX_IQUAL_NONE;

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_LEAF:
    case OBJ_TYP_CHOICE:
    case OBJ_TYP_CASE:
    case OBJ_TYP_RPCIO:
	ret = (required) ? NCX_IQUAL_ONE : NCX_IQUAL_OPT;
	break;
    case OBJ_TYP_LEAF_LIST:
	if (obj->def.leaflist->minset) {
	    if (obj->def.leaflist->maxset && 
		obj->def.leaflist->maxelems==1) {
		ret = NCX_IQUAL_ONE;
	    } else {
		ret = NCX_IQUAL_1MORE;
	    }
	} else {
	    if (obj->def.leaflist->maxset && 
		obj->def.leaflist->maxelems==1) {
		ret = NCX_IQUAL_OPT;
	    } else {
		ret = NCX_IQUAL_ZMORE;
	    }
	}
	break;
    case OBJ_TYP_LIST:
	if (obj->def.list->minset) {
	    if (obj->def.list->maxset && obj->def.list->maxelems==1) {
		ret = NCX_IQUAL_ONE;
	    } else {
		ret = NCX_IQUAL_1MORE;
	    }
	} else {
	    if (obj->def.list->maxset && obj->def.list->maxelems==1) {
		ret = NCX_IQUAL_OPT;
	    } else {
		ret = NCX_IQUAL_ZMORE;
	    }
	}
	break;
    case OBJ_TYP_REFINE:
	ret = NCX_IQUAL_ZMORE;
	break;
    case OBJ_TYP_RPC:
    case OBJ_TYP_NOTIF:
	ret = NCX_IQUAL_ONE;
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }
    return ret;

}  /* obj_get_iqualval_ex */


/********************************************************************
* FUNCTION obj_get_min_elements
* 
* Get the min-elements clause for this object, if any
*
* INPUTS:
*    obj  == object to check
*    minelems == address of return min-elements value
*
* OUTPUTS:
*   *minelems == min-elements value if it is set for this object
*   
* RETURNS:
*    TRUE if min-elements is set, FALSE if not or N/A
*********************************************************************/
boolean
    obj_get_min_elements (obj_template_t  *obj,
			  uint32 *minelems)
{

#ifdef DEBUG
    if (!obj || !minelems) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_LEAF_LIST:
	*minelems = obj->def.leaflist->minelems;
	return obj->def.leaflist->minset;
    case OBJ_TYP_LIST:
	*minelems = obj->def.list->minelems;
	return obj->def.list->minset;
    case OBJ_TYP_REFINE:
	*minelems = obj->def.refine->minelems;
	return (obj->def.refine->minelems_tkerr.mod) ? TRUE : FALSE;
    default:
	return FALSE;
    }
    /*NOTREACHED*/

}  /* obj_get_min_elements */


/********************************************************************
* FUNCTION obj_get_max_elements
* 
* Get the max-elements clause for this object, if any
*
* INPUTS:
*    obj  == object to check
*    maxelems == address of return max-elements value
*
* OUTPUTS:
*   *maxelems == max-elements value if it is set for this object
*
* RETURNS:
*    TRUE if max-elements is set, FALSE if not or N/A
*********************************************************************/
boolean
    obj_get_max_elements (obj_template_t  *obj,
			  uint32 *maxelems)
{

#ifdef DEBUG
    if (!obj || !maxelems) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_LEAF_LIST:
	*maxelems = obj->def.leaflist->maxelems;
	return obj->def.leaflist->maxset;
    case OBJ_TYP_LIST:
	*maxelems = obj->def.list->maxelems;
	return obj->def.list->maxset;
    case OBJ_TYP_REFINE:
	*maxelems = obj->def.refine->maxelems;
	return (obj->def.refine->maxelems_tkerr.mod) ? TRUE : FALSE;
    default:
	return FALSE;
    }
    /*NOTREACHED*/

}  /* obj_get_max_elements */


/********************************************************************
* FUNCTION obj_get_units
* 
* Get the units clause for this object, if any
*
* INPUTS:
*    obj  == object to check
*
* RETURNS:
*    pointer to units clause, or NULL if none
*********************************************************************/
const xmlChar *
    obj_get_units (obj_template_t  *obj)
{
    const xmlChar    *units;
    const typ_def_t  *typdef;

#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    units = NULL;

    switch (obj->objtype) {
    case OBJ_TYP_LEAF:
	units = obj->def.leaf->units;
	break;
    case OBJ_TYP_LEAF_LIST:
	units = obj->def.leaflist->units;
	break;
    default:
	return NULL;
    }

    if (!units) {
	typdef = obj_get_ctypdef(obj);
	if (typdef) {
	    units = typ_get_units_from_typdef(typdef);
	}
    }
    return units;

}  /* obj_get_units */


/********************************************************************
* FUNCTION obj_get_parent
* 
* Get the parent of the current object
*
* INPUTS:
*    obj  == object to check
*
* RETURNS:
*    pointer to the parent of this object or NULL if none
*********************************************************************/
obj_template_t *
    obj_get_parent (obj_template_t  *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return obj->parent;

}  /* obj_get_parent */


/********************************************************************
* FUNCTION obj_get_cparent
* 
* Get the parent of the current object
* CONST POINTER VERSION
*
* INPUTS:
*    obj  == object to check
*
* RETURNS:
*    pointer to the parent of this object or NULL if none
*********************************************************************/
const obj_template_t *
    obj_get_cparent (const obj_template_t  *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return obj->parent;

}  /* obj_get_cparent */


/********************************************************************
* FUNCTION obj_is_leafy
* 
* Check if object is a proper leaf or leaflist 
*
* INPUTS:
*    obj  == object to check
*
* RETURNS:
*    TRUE if proper leaf or leaf-list
*    FALSE if node
*********************************************************************/
boolean
    obj_is_leafy (const obj_template_t  *obj)
{
    if (obj->objtype == OBJ_TYP_LEAF ||
	obj->objtype == OBJ_TYP_LEAF_LIST) {
	return TRUE;
    } else {
	return FALSE;
    }
    /*NOTREACHED*/

}  /* obj_is_leafy */


/********************************************************************
* FUNCTION obj_is_mandatory
*
* Figure out if the obj is YANG mandatory or not
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is mandatory
*   FALSE if object is not mandatory
*********************************************************************/
boolean
    obj_is_mandatory (obj_template_t *obj)
{
    obj_template_t *chobj;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	if (obj->def.container->presence) {
	    return FALSE;
	}
	/* else drop through and check children */
    case OBJ_TYP_CASE:
    case OBJ_TYP_RPCIO:
	for (chobj = obj_first_child(obj);
	     chobj != NULL;
	     chobj = obj_next_child(chobj)) {
	    if (obj_is_mandatory(chobj)) {
		return TRUE;
	    }
	}
	return FALSE;
    case OBJ_TYP_LEAF:
	if (obj_is_key(obj)) {
	    return TRUE;
	} 
	/* else fall through */
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_CHOICE:
	return (obj->flags & OBJ_FL_MANDATORY) ? TRUE : FALSE;
    case OBJ_TYP_LEAF_LIST:
	return (obj->def.leaflist->minelems) ? TRUE : FALSE;
    case OBJ_TYP_LIST:
	return (obj->def.list->minelems) ? TRUE : FALSE;
    case OBJ_TYP_USES:
    case OBJ_TYP_AUGMENT:
    case OBJ_TYP_REFINE:
    case OBJ_TYP_RPC:
    case OBJ_TYP_NOTIF:
	return FALSE;
    case OBJ_TYP_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

}   /* obj_is_mandatory */


/********************************************************************
* FUNCTION obj_is_mandatory_when
*
* Figure out if the obj is YANG mandatory or not
* Check the when-stmts, not just mandatory-stmt
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is mandatory
*   FALSE if object is not mandatory
*********************************************************************/
boolean
    obj_is_mandatory_when (obj_template_t *obj)
{
    obj_template_t *chobj;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	if (obj->def.container->presence) {
	    return FALSE;
	}
	/* else drop through and check children */
    case OBJ_TYP_CASE:
    case OBJ_TYP_RPCIO:
	for (chobj = obj_first_child(obj);
	     chobj != NULL;
	     chobj = obj_next_child(chobj)) {
	    if (obj_is_mandatory_when(chobj)) {
		return TRUE;
	    }
	}
	return FALSE;
    case OBJ_TYP_LEAF:
	if (obj_is_key(obj)) {
	    return TRUE;
	} 
	/* else fall through */
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_CHOICE:
        if (obj_has_when_stmts(obj)) {
            return FALSE;
        }
	return (obj->flags & OBJ_FL_MANDATORY) ? TRUE : FALSE;
    case OBJ_TYP_LEAF_LIST:
        if (obj_has_when_stmts(obj)) {
            return FALSE;
        }
	return (obj->def.leaflist->minelems) ? TRUE : FALSE;
    case OBJ_TYP_LIST:
        if (obj_has_when_stmts(obj)) {
            return FALSE;
        }
	return (obj->def.list->minelems) ? TRUE : FALSE;
    case OBJ_TYP_USES:
    case OBJ_TYP_AUGMENT:
    case OBJ_TYP_REFINE:
    case OBJ_TYP_RPC:
    case OBJ_TYP_NOTIF:
	return FALSE;
    case OBJ_TYP_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

}   /* obj_is_mandatory_when */


/********************************************************************
* FUNCTION obj_is_cloned
*
* Figure out if the obj is a cloned object, inserted via uses
* or augment statements
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is cloned
*   FALSE if object is not cloned
*********************************************************************/
boolean
    obj_is_cloned (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return (obj->flags & OBJ_FL_CLONE) ? TRUE : FALSE;

}   /* obj_is_cloned */


/********************************************************************
* FUNCTION obj_is_augclone
*
* Figure out if the obj is a cloned object, inserted via an
* augment statement
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is sourced from an augment
*   FALSE if object is not sourced from an augment
*********************************************************************/
boolean
    obj_is_augclone (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return (obj->flags & OBJ_FL_AUGCLONE) ? TRUE : FALSE;

}   /* obj_is_augclone */


/********************************************************************
* FUNCTION obj_is_refine
*
* Figure out if the obj is a refinement object, within a uses-stmt
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is a refinement
*   FALSE if object is not a refinement
*********************************************************************/
boolean
    obj_is_refine (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return (obj->objtype == OBJ_TYP_REFINE) ? TRUE : FALSE;

}   /* obj_is_refine */


/********************************************************************
* FUNCTION obj_is_data
* 
* Check if the object is defined within data or within a
* notification or RPC instead
* 
* INPUTS:
*   obj == object to check
*  
* RETURNS:
*   TRUE if data object (could be in a grouping or real data)
*   FALSE if defined within notification or RPC (or some error)
*********************************************************************/
boolean
    obj_is_data (const obj_template_t *obj)
{
#ifdef DEBUG 
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_RPC:
    case OBJ_TYP_NOTIF:
	return FALSE;
    case OBJ_TYP_RPCIO:
	return TRUE;  /* hack for yangdump HTML output */
    case OBJ_TYP_REFINE:
	return FALSE;
    default:
	if (obj->parent && !obj_is_root(obj->parent)) {
	    return obj_is_data(obj->parent);
	} else {
	    return TRUE;
	}
    }
    /*NOTREACHED*/

}  /* obj_is_data */


/********************************************************************
* FUNCTION obj_is_data_db
* 
* Check if the object is some sort of data
* Constrained to only check the config DB objects,
* not any notification or RPC objects
*
* INPUTS:
*   obj == object to check
*  
* RETURNS:
*   TRUE if data object (could be in a grouping or real data)
*   FALSE if defined within notification or RPC (or some error)
*********************************************************************/
boolean
    obj_is_data_db (const obj_template_t *obj)
{
#ifdef DEBUG 
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    if (obj_is_abstract(obj) || obj_is_cli(obj)) {
        return FALSE;
    }

    switch (obj->objtype) {
    case OBJ_TYP_RPC:
    case OBJ_TYP_NOTIF:
	return FALSE;
    case OBJ_TYP_RPCIO:
	return FALSE;
    case OBJ_TYP_REFINE:
	return FALSE;
    default:
	if (obj_is_root(obj)) {
	    return TRUE;
	} else if (obj->parent && !obj_is_root(obj->parent)) {
	    return obj_is_data_db(obj->parent);
	} else {
	    return TRUE;
	}
    }
    /*NOTREACHED*/

}  /* obj_is_data_db */


/********************************************************************
* FUNCTION obj_in_rpc
* 
* Check if the object is in an rpc/input section
*
* INPUTS:
*   obj == object to check
*  
* RETURNS:
*   TRUE if /rpc/input object
*   FALSE otherwise
*********************************************************************/
boolean
    obj_in_rpc (const obj_template_t *obj)
{
#ifdef DEBUG 
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_RPC:
    case OBJ_TYP_NOTIF:
	return FALSE;
    case OBJ_TYP_RPCIO:
	return (!xml_strcmp(obj_get_name(obj), YANG_K_INPUT)) ?
	    TRUE : FALSE;
    case OBJ_TYP_REFINE:
	return FALSE;
    default:
	if (obj->parent && !obj_is_root(obj->parent)) {
	    return obj_in_rpc(obj->parent);
	} else {
	    return FALSE;
	}
    }
    /*NOTREACHED*/

}  /* obj_in_rpc */


/********************************************************************
* FUNCTION obj_in_rpc_reply
* 
* Check if the object is in an rpc-reply/output section
*
* INPUTS:
*   obj == object to check
*  
* RETURNS:
*   TRUE if /rpc-reply/output object
*   FALSE otherwise
*********************************************************************/
boolean
    obj_in_rpc_reply (const obj_template_t *obj)
{
#ifdef DEBUG 
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_RPC:
    case OBJ_TYP_NOTIF:
	return FALSE;
    case OBJ_TYP_RPCIO:
	return (!xml_strcmp(obj_get_name(obj), YANG_K_OUTPUT)) ?
	    TRUE : FALSE;
    case OBJ_TYP_REFINE:
	return FALSE;
    default:
	if (obj->parent && !obj_is_root(obj->parent)) {
	    return obj_in_rpc_reply(obj->parent);
	} else {
	    return FALSE;
	}
    }
    /*NOTREACHED*/

}  /* obj_in_rpc_reply */


/********************************************************************
* FUNCTION obj_in_notif
* 
* Check if the object is in a notification
*
* INPUTS:
*   obj == object to check
*  
* RETURNS:
*   TRUE if /notification object
*   FALSE otherwise
*********************************************************************/
boolean
    obj_in_notif (const obj_template_t *obj)
{
#ifdef DEBUG 
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_RPC:
	return FALSE;
    case OBJ_TYP_NOTIF:
	return TRUE;
    case OBJ_TYP_RPCIO:
	return FALSE;
    case OBJ_TYP_REFINE:
	return FALSE;
    default:
	if (obj->parent && !obj_is_root(obj->parent)) {
	    return obj_in_notif(obj->parent);
	} else {
	    return FALSE;
	}
    }
    /*NOTREACHED*/

}  /* obj_in_notif */


/********************************************************************
* FUNCTION obj_is_rpc
* 
* Check if the object is an RPC method
* 
* INPUTS:
*   obj == object to check
*  
* RETURNS:
*   TRUE if RPC method
*   FALSE if not an RPC method
*********************************************************************/
boolean
    obj_is_rpc (const obj_template_t *obj)
{
#ifdef DEBUG 
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    return (obj->objtype == OBJ_TYP_RPC) ? TRUE : FALSE;

}  /* obj_is_rpc */


/********************************************************************
* FUNCTION obj_is_notif
* 
* Check if the object is a notification
* 
* INPUTS:
*   obj == object to check
*  
* RETURNS:
*   TRUE if notification
*   FALSE if not
*********************************************************************/
boolean
    obj_is_notif (const obj_template_t *obj)
{
#ifdef DEBUG 
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    return (obj->objtype == OBJ_TYP_NOTIF) ? TRUE : FALSE;

}  /* obj_is_notif */


/********************************************************************
* FUNCTION obj_is_empty
*
* Check if object was entered in empty fashion:
*   list foo;
*   uses grpx;
*
** INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is empty of subclauses
*   FALSE if object is not empty of subclauses
*********************************************************************/
boolean
    obj_is_empty (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return (obj->flags & OBJ_FL_EMPTY) ? TRUE : FALSE;

}   /* obj_is_empty */


/********************************************************************
* FUNCTION obj_is_match
* 
* Check if one object is a match in identity with another one
*
* INPUTS:
*    obj1  == first object to match
*    obj2  == second object to match
*
* RETURNS:
*    TRUE is a match, FALSE otherwise
*********************************************************************/
boolean
    obj_is_match (const obj_template_t  *obj1,
		  const obj_template_t *obj2)
{
    if (xml_strcmp(obj_get_mod_name(obj1),
		   obj_get_mod_name(obj2))) {
	return FALSE;
    }

    if (obj_has_name(obj1) && obj_has_name(obj2)) {
	return xml_strcmp(obj_get_name(obj1), 
			  obj_get_name(obj2)) ? FALSE : TRUE;
    } else {
	return FALSE;
    }

}  /* obj_is_match */


/********************************************************************
* FUNCTION obj_set_ncx_flags
*
* Check the NCX appinfo extensions and set flags as needed
*
** INPUTS:
*   obj == obj_template to check
*
* OUTPUTS:
*   may set additional bits in the obj->flags field
*
*********************************************************************/
void
    obj_set_ncx_flags (obj_template_t *obj)
{

    const dlq_hdr_t  *appinfoQ;
    const typ_def_t  *typdef;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    appinfoQ = obj_get_appinfoQ(obj);

    if (ncx_find_const_appinfo(appinfoQ, 
                               NCX_PREFIX, 
                               NCX_EL_PASSWORD)) {
	obj->flags |= OBJ_FL_PASSWD;
    }

    if (ncx_find_const_appinfo(appinfoQ, 
                               NCX_PREFIX, 
                               NCX_EL_HIDDEN)) {
	obj->flags |= OBJ_FL_HIDDEN;
    }

    if (ncx_find_const_appinfo(appinfoQ, 
                               NCX_PREFIX, 
                               NCX_EL_XSDLIST)) {
	obj->flags |= OBJ_FL_XSDLIST;
    }

    if (ncx_find_const_appinfo(appinfoQ, 
                               NCX_PREFIX, 
                               NCX_EL_ROOT)) {
	obj->flags |= OBJ_FL_ROOT;
    }

    if (ncx_find_const_appinfo(appinfoQ, 
                               NCX_PREFIX, 
                               NCX_EL_CLI)) {
	obj->flags |= OBJ_FL_CLI;
    }

    if (ncx_find_const_appinfo(appinfoQ, 
                               NCX_PREFIX, 
                               NCX_EL_ABSTRACT)) {
	obj->flags |= OBJ_FL_ABSTRACT;
    }

    if (ncx_find_const_appinfo(appinfoQ, 
                               NCX_PREFIX, 
                               NCX_EL_SECURE)) {
	obj->flags |= OBJ_FL_SECURE;
    }

    if (ncx_find_const_appinfo(appinfoQ, 
                               NCX_PREFIX, 
                               NCX_EL_VERY_SECURE)) {
	obj->flags |= OBJ_FL_VERY_SECURE;
    }

    if (obj_is_leafy(obj)) {
	typdef = obj_get_ctypdef(obj);

	/* ncx:xpath extension */
	if (typ_is_xpath_string(typdef)) {
	    obj->flags |= OBJ_FL_XPATH;
	} else if (ncx_find_const_appinfo(appinfoQ, 
                                          NCX_PREFIX, 
                                          NCX_EL_XPATH)) {
	    obj->flags |= OBJ_FL_XPATH;
	}

	/* ncx:qname extension */
	if (typ_is_qname_string(typdef)) {
	    obj->flags |= OBJ_FL_QNAME;
	} else if (ncx_find_const_appinfo(appinfoQ, 
                                          NCX_PREFIX, 
                                          NCX_EL_XPATH)) {
	    obj->flags |= OBJ_FL_QNAME;
	}

	/* ncx:schema-instance extension */
	if (typ_is_schema_instance_string(typdef)) {
	    obj->flags |= OBJ_FL_SCHEMAINST;
	} else if (ncx_find_const_appinfo(appinfoQ, 
                                          NCX_PREFIX, 
                                          NCX_EL_SCHEMA_INSTANCE)) {
	    obj->flags |= OBJ_FL_SCHEMAINST;
	}
    }

}   /* obj_set_ncx_flags */


/********************************************************************
* FUNCTION obj_is_hidden
*
* Check if object is marked as a hidden object
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is marked as ncx:hidden
*   FALSE if not
*********************************************************************/
boolean
    obj_is_hidden (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return (obj->flags & OBJ_FL_HIDDEN) ? TRUE : FALSE;

}   /* obj_is_hidden */


/********************************************************************
* FUNCTION obj_is_root
*
* Check if object is marked as a root object
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is marked as ncx:root
*   FALSE if not
*********************************************************************/
boolean
    obj_is_root (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return (obj->flags & OBJ_FL_ROOT) ? TRUE : FALSE;

}   /* obj_is_root */


/********************************************************************
* FUNCTION obj_is_password
*
* Check if object is marked as a password object
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is marked as ncx:password
*   FALSE if not
*********************************************************************/
boolean
    obj_is_password (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return (obj->flags & OBJ_FL_PASSWD) ? TRUE : FALSE;

}   /* obj_is_password */


/********************************************************************
* FUNCTION obj_is_xsdlist
*
* Check if object is marked as an XSD list
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is marked as ncx:xsdlist
*   FALSE if not
*********************************************************************/
boolean
    obj_is_xsdlist (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return (obj->flags & OBJ_FL_XSDLIST) ? TRUE : FALSE;

}   /* obj_is_xsdlist */


/********************************************************************
* FUNCTION obj_is_cli
*
* Check if object is marked as a CLI object
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is marked as ncx:cli
*   FALSE if not
*********************************************************************/
boolean
    obj_is_cli (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return (obj->flags & OBJ_FL_CLI) ? TRUE : FALSE;

}   /* obj_is_cli */


/********************************************************************
* FUNCTION obj_is_key
*
* Check if object is being used as a key leaf within a list
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is a key leaf
*   FALSE if not
*********************************************************************/
boolean
    obj_is_key (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return (obj->flags & OBJ_FL_KEY) ? TRUE : FALSE;

}   /* obj_is_key */


/********************************************************************
* FUNCTION obj_is_abstract
*
* Check if object is being used as an object identifier or error-info
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is marked as ncx:abstract
*   FALSE if not
*********************************************************************/
boolean
    obj_is_abstract (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return (obj->flags & OBJ_FL_ABSTRACT) ? TRUE : FALSE;

}   /* obj_is_abstract */


/********************************************************************
* FUNCTION obj_is_xpath_string
*
* Check if object is an XPath string
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is marked as ncx:xpath
*   FALSE if not
*********************************************************************/
boolean
    obj_is_xpath_string (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return ((obj->flags & (OBJ_FL_XPATH | OBJ_FL_SCHEMAINST)) ||
	    obj_get_basetype(obj)==NCX_BT_INSTANCE_ID) 
	? TRUE : FALSE;

}   /* obj_is_xpath_string */


/********************************************************************
* FUNCTION obj_is_schema_instance_string
*
* Check if object is a schema-instance string
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is marked as ncx:schema-instance
*   FALSE if not
*********************************************************************/
boolean
    obj_is_schema_instance_string (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    if (obj_get_basetype(obj) != NCX_BT_STRING) {
	return FALSE;
    }

    return (obj->flags & OBJ_FL_SCHEMAINST)
	? TRUE : FALSE;

}   /* obj_is_schema_instance_string */


/********************************************************************
* FUNCTION obj_is_secure
*
* Check if object is tagged ncx:secure
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is marked as ncx:secure
*   FALSE if not
*********************************************************************/
boolean
    obj_is_secure (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return (obj->flags & OBJ_FL_SECURE)
	? TRUE : FALSE;

}   /* obj_is_secure */


/********************************************************************
* FUNCTION obj_is_very_secure
*
* Check if object is tagged ncx:very-secure
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is marked as ncx:very-secure
*   FALSE if not
*********************************************************************/
boolean
    obj_is_very_secure (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return (obj->flags & OBJ_FL_VERY_SECURE)
	? TRUE : FALSE;

}   /* obj_is_very_secure */


/********************************************************************
* FUNCTION obj_is_system_ordered
*
* Check if the object is system or user-ordered
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is system ordered
*   FALSE if object is user-ordered
*********************************************************************/
boolean
    obj_is_system_ordered (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_LEAF_LIST:
	return obj->def.leaflist->ordersys;
    case OBJ_TYP_LIST:
	return obj->def.list->ordersys;
    default:
	return TRUE;
    }
    /*NOTREACHED*/

}  /* obj_is_system_ordered */


/********************************************************************
* FUNCTION obj_is_np_container
*
* Check if the object is an NP-container
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is an NP-container
*   FALSE if object is not an NP-container
*********************************************************************/
boolean
    obj_is_np_container (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    if (obj->objtype != OBJ_TYP_CONTAINER) {
	return FALSE;
    }

    return (obj->def.container->presence) ? FALSE : TRUE;

}  /* obj_is_np_container */


/********************************************************************
* FUNCTION obj_get_presence_string
*
* Get the present-stmt value, if any
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   pointer to string
*   NULL if none
*********************************************************************/
const xmlChar *
    obj_get_presence_string (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (obj->objtype != OBJ_TYP_CONTAINER) {
	return NULL;
    }

    return obj->def.container->presence;

}  /* obj_get_presence_string */


/********************************************************************
* FUNCTION obj_ok_for_cli
*
* Figure out if the obj is OK for current CLI implementation
* Top object must be a container
* Child objects must be only choices of leafs,
* plain leafs, or leaf lists are allowed
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if object is OK for CLI
*   FALSE if object is not OK for CLI
*********************************************************************/
boolean
    obj_ok_for_cli (obj_template_t *obj)
{
    obj_template_t *chobj, *casobj, *caschild;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    if (obj->objtype != OBJ_TYP_CONTAINER) {
	return FALSE;
    }

    for (chobj = obj_first_child(obj);
	 chobj != NULL;
	 chobj = obj_next_child(chobj)) {

	switch (chobj->objtype) {
	case OBJ_TYP_ANYXML:
	    return TRUE;   /**** was FALSE ****/
	case OBJ_TYP_LEAF:
	case OBJ_TYP_LEAF_LIST:
	    break;
	case OBJ_TYP_CHOICE:
	    for (casobj = obj_first_child(chobj);
		 casobj != NULL;
		 casobj = obj_next_child(casobj)) {

		for (caschild = obj_first_child(casobj);
		     caschild != NULL;
		     caschild = obj_next_child(caschild)) {
		    switch (caschild->objtype) {
		    case OBJ_TYP_ANYXML:
			return FALSE;
		    case OBJ_TYP_LEAF:
		    case OBJ_TYP_LEAF_LIST:
			break;
		    default:
			return FALSE;
		    }
		}
	    }
	    break;
	default:
	    return FALSE;
	}
    }

    return TRUE;

}   /* obj_ok_for_cli */


/********************************************************************
 * FUNCTION obj_get_child_node
 * 
 * Get the correct child node for the specified parent and
 * current XML node
 *
 * INPUTS:
 *    obj == parent object template
 *    chobj == current child node (may be NULL if the
 *             xmlorder param is FALSE
 *     xmlorder == TRUE if should follow strict XML element order
 *              == FALSE if sibling node order errors should be 
 *                 ignored; find child nodes out of order
 *                 and check too-many-instances later
 *    curnode == current XML start or empty node to check
 *    force_modQ == Q of ncx_module_t to check, if set
 *               == NULL and the xmlns registry of module pointers
 *                  will be used instead (except netconf.yang)
 *    rettop == address of return topchild object
 *    retobj == address of return object to use
 *
 * OUTPUTS:
 *    *rettop set to top-level found object if return OK
 *     and currently within a choice
 *    *retobj set to found object if return OK
 *
 * RETURNS:
 *   status
 *********************************************************************/
status_t 
    obj_get_child_node (obj_template_t *obj,
			obj_template_t *chobj,
			const xml_node_t *curnode,
			boolean xmlorder,
                        dlq_hdr_t *force_modQ,
			obj_template_t **rettop,
			obj_template_t **retobj)
{
    obj_template_t        *foundobj, *nextchobj;
    const xmlChar         *foundmodname;
    ncx_module_t          *foundmod;
    status_t               res;
    ncx_node_t             dtyp;
    boolean                topdone;
    xmlns_id_t             ncnid, ncid;

#ifdef DEBUG
    if (!obj || !curnode || !rettop || !retobj) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    foundobj = NULL;
    foundmodname = NULL;
    foundmod = NULL;
    dtyp = NCX_NT_OBJ;
    res = NO_ERR;
    topdone = FALSE;
    ncid = xmlns_nc_id();
    ncnid = xmlns_ncn_id();

    if (curnode->nsid) {
        if (curnode->nsid == ncid || force_modQ == NULL) {
            foundmod = xmlns_get_modptr(curnode->nsid);
        } else if (force_modQ) {
            foundmod = ncx_find_module_que_nsid(force_modQ,
                                                curnode->nsid);
        }
        if (foundmod) {
            foundmodname = ncx_get_modname(foundmod);
        }
    } 

    if (obj_is_root(obj)) {
	/* the child node can be any top-level object
	 * in the configuration database
	 */
	if (foundmodname) {
	    /* get the name from 1 module */
	    foundobj =  ncx_find_object(foundmod,
					curnode->elname);
	} else if (force_modQ) {
            /* check this Q of modules for a top-level match */
	    foundobj = ncx_find_any_object_que(force_modQ,
                                               curnode->elname);
	    if (foundobj) {
		foundmodname = obj_get_mod_name(foundobj);
	    }
        } else {
	    /* NSID not set, get the name from any module */
	    foundobj = ncx_find_any_object(curnode->elname);
	    if (foundobj) {
		foundmodname = obj_get_mod_name(foundobj);
	    }
	}

	if (foundobj) {
	    if (!obj_is_data_db(foundobj) ||
		obj_is_abstract(foundobj) ||
		obj_is_cli(foundobj)) {
		foundobj = NULL;
	    }
	}
    } else if (obj_get_nsid(obj) == ncnid &&
	       !xml_strcmp(obj_get_name(obj),
			   NCX_EL_NOTIFICATION)) {
	/* hack: special case handling of the
	 * <notification> element
	 * the child node can be <eventTime> or
	 * any top-level OBJ_TYP_NOTIF node
	 */
	if (foundmodname) {
            /* try a child of <notification> */
            foundobj = obj_find_child(obj, 
                                      foundmodname,
                                      curnode->elname);
            if (!foundobj) {
                /* try to find an <eventType> */
		foundobj =  ncx_find_object(foundmod,
					    curnode->elname);
		if (foundobj && 
		    foundobj->objtype != OBJ_TYP_NOTIF) {
		    /* object is the wrong type */
		    foundobj = NULL;
		}
	    }
	} else {
	    /* no namespace ID used
	     * try to find any eventType object
	     */
            if (force_modQ) {
                foundobj = ncx_find_any_object_que(force_modQ,
                                                   curnode->elname);
            } else {
                foundobj = ncx_find_any_object(curnode->elname);
            }
	    if (foundobj) {
		if (foundobj->objtype != OBJ_TYP_NOTIF) {
		    foundobj = NULL;
		}
	    } else {
		/* try a child of obj (eventTime) */
		foundobj = obj_find_child(obj, 
					  NULL,
					  curnode->elname);
	    }
	}
    } else if (xmlorder) {
	/* the current node must match or one of the
	 * subsequent child nodes must match
	 */
	if (chobj) {
	    switch (chobj->objtype) {
	    case OBJ_TYP_CHOICE:
	    case OBJ_TYP_CASE:
		/* these nodes are not really in the XML so
		 * check all the child nodes of the
		 * cases.  When found, need to remember
		 * the current child node at the choice
		 * or case level, so when the lower
		 * level child pointer runs out, the
		 * search can continue at the next
		 * sibling of 'rettop'
		 */
		foundobj = obj_find_child(chobj,
					  foundmodname,
					  curnode->elname);
		if (foundobj) {
		    /* make sure this matched a real node instead
		     * of match of choice or case name
		     */
		    if (foundobj->objtype==OBJ_TYP_CHOICE ||
			foundobj->objtype==OBJ_TYP_CASE) {
			foundobj = NULL;
		    } else {
			*rettop = chobj;
			topdone = TRUE;
		    }
		}		    
		break;
	    default:
		/* the YANG node and XML node line up,
		 * so it is OK to compare them directly
		 */
		res = xml_node_match(curnode, 
				     obj_get_nsid(chobj), 
				     obj_get_name(chobj), 
				     XML_NT_NONE);

		if (res == NO_ERR) {
		    foundobj = chobj;
		} else {
		    foundobj = NULL;
		}
	    }

	    if (!foundobj) {
		/* check if there are other child nodes that could
		 * match, due to instance qualifiers 
		 */
		nextchobj = find_next_child(chobj, curnode);
		if (nextchobj) {
		    res = NO_ERR;
		    foundobj = nextchobj;
		} else if (*rettop) {
		    nextchobj = find_next_child(*rettop, curnode);
		    if (nextchobj) {
			res = NO_ERR;
			foundobj = nextchobj;
		    }
		}
	    }
	}
    } else {
	/* do not care about XML order, just match any node
	 * within the current parent object
	 */
	if (curnode->nsid) {
	    /* find the specified module first */

	    if (foundmodname) {
		foundobj = obj_find_child(obj, 
					  foundmodname,
					  curnode->elname);
	    }
	} else {
	    /* get the object from first match module */
	    foundobj = obj_find_child(obj, 
				      NULL,
				      curnode->elname);
	}
    }

    if (foundobj) {
	*retobj = foundobj;
	if (!topdone) {
	    *rettop = foundobj;
	}
	return NO_ERR;
    } else if (res != NO_ERR) {
	return res;
    } else {
	return ERR_NCX_DEF_NOT_FOUND;
    }
    /*NOTREACHED*/

}  /* obj_get_child_node */


/********************************************************************
* FUNCTION obj_get_child_count
*
* Get the number of child nodes the object has
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   number of child nodes
*********************************************************************/
uint32
    obj_get_child_count (const obj_template_t *obj)
{

    const dlq_hdr_t   *datadefQ;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return 0;
    }
#endif

    datadefQ = obj_get_cdatadefQ(obj);
    if (datadefQ) {
	return dlq_count(datadefQ);
    } else {
	return 0;
    }

}   /* obj_get_child_count */


/********************************************************************
* FUNCTION obj_has_children
*
* Check if there are any accessible nodes within the object
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if there are any accessible children
*   FALSE if no datadb child nodes found
*********************************************************************/
boolean
    obj_has_children (obj_template_t *obj)
{
    const obj_template_t *childobj;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return 0;
    }
#endif

    childobj = obj_first_child_deep(obj);
    if (childobj) {
	return TRUE;
    } else {
	return FALSE;
    }

}   /* obj_has_children */


/********************************************************************
* FUNCTION obj_new_metadata
* 
* Malloc and initialize the fields in a an obj_metadata_t
*
* INPUTS:
*  isreal == TRUE if this is for a real object
*          == FALSE if this is a cloned object
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
obj_metadata_t * 
    obj_new_metadata (void)
{
    obj_metadata_t  *meta;

    meta = m__getObj(obj_metadata_t);
    if (!meta) {
	return NULL;
    }

    (void)memset(meta, 0x0, sizeof(obj_metadata_t));

    meta->typdef = typ_new_typdef();
    if (!meta->typdef) {
	m__free(meta);
	return NULL;
    }

    return meta;

}  /* obj_new_metadata */


/********************************************************************
* FUNCTION obj_free_metadata
* 
* Scrub the memory in a obj_metadata_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    meta == obj_metadata_t data structure to free
*********************************************************************/
void 
    obj_free_metadata (obj_metadata_t *meta)
{
#ifdef DEBUG
    if (!meta) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (meta->name) {
	m__free(meta->name);
    }
    if (meta->typdef) {
	typ_free_typdef(meta->typdef);
    }
    m__free(meta);

}  /* obj_free_metadata */


/********************************************************************
* FUNCTION obj_add_metadata
* 
* Add the filled out object metadata definition to the object
*
* INPUTS:
*    meta == obj_metadata_t data structure to add
*    obj == object template to add meta to
*
* RETURNS:
*    status
*********************************************************************/
status_t
    obj_add_metadata (obj_metadata_t *meta,
		      obj_template_t *obj)
{
    obj_metadata_t *testmeta;

#ifdef DEBUG
    if (!meta || !obj) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    testmeta = obj_find_metadata(obj, meta->name);
    if (testmeta) {
	return ERR_NCX_ENTRY_EXISTS;
    }

    meta->parent = obj;
    meta->nsid = obj_get_nsid(obj);
    dlq_enque(meta, &obj->metadataQ);
    return NO_ERR;

}  /* obj_add_metadata */


/********************************************************************
* FUNCTION obj_find_metadata
* 
* Find the object metadata definition in the object
*
* INPUTS:
*    obj == object template to check
*    name == name of obj_metadata_t data structure to find
*
* RETURNS:
*    pointer to found entry, NULL if not found
*********************************************************************/
obj_metadata_t *
    obj_find_metadata (const obj_template_t *obj,
		       const xmlChar *name)
{
    obj_metadata_t *testmeta;

#ifdef DEBUG
    if (!obj || !name) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (testmeta = (obj_metadata_t *)
	     dlq_firstEntry(&obj->metadataQ);
	 testmeta != NULL;
	 testmeta = (obj_metadata_t *)
	     dlq_nextEntry(testmeta)) {

	if (!xml_strcmp(testmeta->name, name)) {
	    return testmeta;
	}
    }

    return NULL;

}  /* obj_find_metadata */


/********************************************************************
* FUNCTION obj_first_metadata
* 
* Get the first object metadata definition in the object
*
* INPUTS:
*    obj == object template to check
*
* RETURNS:
*    pointer to first entry, NULL if none
*********************************************************************/
obj_metadata_t *
    obj_first_metadata (const obj_template_t *obj)
{

#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (obj_metadata_t *)
	dlq_firstEntry(&obj->metadataQ);

}  /* obj_first_metadata */


/********************************************************************
* FUNCTION obj_next_metadata
* 
* Get the next object metadata definition in the object
*
* INPUTS:
*    meta == current meta object template
*
* RETURNS:
*    pointer to next entry, NULL if none
*********************************************************************/
obj_metadata_t *
    obj_next_metadata (const obj_metadata_t *meta)
{

#ifdef DEBUG
    if (!meta) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (obj_metadata_t *)dlq_nextEntry(meta);

}  /* obj_next_metadata */


/********************************************************************
* FUNCTION obj_get_default_parm
* 
* Get the ncx:default-parm object for this object
* Only supported for OBJ_TYP_CONTAINER and OBJ_TYP_RPCIO (input)
*
* INPUTS:
*   obj == the specific object to check
*
* RETURNS:
*   pointer to the name field, NULL if some error or unnamed
*********************************************************************/
obj_template_t * 
    obj_get_default_parm (obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	return obj->def.container->defaultparm;
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
    case OBJ_TYP_LIST:
    case OBJ_TYP_CHOICE:
    case OBJ_TYP_CASE:
    case OBJ_TYP_USES:
    case OBJ_TYP_AUGMENT:
    case OBJ_TYP_REFINE:
    case OBJ_TYP_RPC:
    case OBJ_TYP_ANYXML:
	return NULL;
    case OBJ_TYP_RPCIO:
	return obj->def.rpcio->defaultparm;
    case OBJ_TYP_NOTIF:
	return NULL;
    case OBJ_TYP_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
    /*NOTREACHED*/

}  /* obj_get_default_parm */


/********************************************************************
* FUNCTION get_config_flag_deep
*
* Get the config flag for an obj_template_t 
* Go all the way up the tree until an explicit
* set node or the root is found
*
* Used by get_list_key because the config flag
* of the parent is not set yet when a key leaf is expanded
*
* INPUTS:
*   obj == obj_template to check
*
* RETURNS:
*   TRUE if config set to TRUE
*   FALSE if config set to FALSE
*********************************************************************/
boolean
    obj_get_config_flag_deep (const obj_template_t *obj)
{
    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
    case OBJ_TYP_ANYXML:
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
    case OBJ_TYP_LIST:
    case OBJ_TYP_CHOICE:
	if (obj_is_root(obj)) {
	    return TRUE;
	}
	/* check if this normal object has a config-stmt */
	if (obj->flags & OBJ_FL_CONFSET) {
	    return (obj->flags & OBJ_FL_CONFIG) ? TRUE : FALSE;
	}

	if (obj->parent) {
	    return obj_get_config_flag_deep(obj->parent);
	}

	/* should not really get here, since all 
	 * top-level objects should have the OBJ_FL_CONFSET
	 * flag set: default ifor top-level is config=true
	 */
	return TRUE;
    case OBJ_TYP_CASE:
	if (obj->parent) {
	    return obj_get_config_flag_deep(obj->parent);
	} else {
	    /* should not happen */
	    return FALSE;
	}
    case OBJ_TYP_USES:
    case OBJ_TYP_AUGMENT:
    case OBJ_TYP_REFINE:
	/* no real setting -- not applicable */
	return FALSE;
    case OBJ_TYP_RPC:
	/* no real setting for this, but has to be true
	 * to allow rpc/input to be true
	 */
	return TRUE;
    case OBJ_TYP_RPCIO:
	if (!xml_strcmp(obj->def.rpcio->name, YANG_K_INPUT)) {
	    return TRUE;
	} else {
	    return FALSE;
	}
    case OBJ_TYP_NOTIF:
	return FALSE;
    case OBJ_TYP_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }
    /*NOTREACHED*/

}   /* obj_get_config_flag_deep */


/********************************************************************
* FUNCTION obj_get_fraction_digits
* 
* Get the fraction-digits field from the object typdef
*
* INPUTS:
*     obj == object template to  check
*
* RETURNS:
*     number of fixed decimal digits expected (1..18)
*     0 if some error
*********************************************************************/
uint8
    obj_get_fraction_digits (const obj_template_t  *obj)
{
    const typ_def_t  *typdef;

#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return 0;
    }
#endif

    typdef = obj_get_ctypdef(obj);
    if (typdef) {
	return typ_get_fraction_digits(typdef);
    } else {
	return 0;
    }

}  /* obj_get_fraction_digits */


/********************************************************************
* FUNCTION obj_get_first_iffeature
* 
* Get the first if-feature clause (if any) for the specified object
*
* INPUTS:
*     obj == object template to  check
*
* RETURNS:
*     pointer to first if-feature struct
*     NULL if none available
*********************************************************************/
const ncx_iffeature_t *
    obj_get_first_iffeature (const obj_template_t  *obj)
{

#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (const ncx_iffeature_t *)
	dlq_firstEntry(&obj->iffeatureQ);

}  /* obj_get_first_iffeature */


/********************************************************************
* FUNCTION obj_get_next_iffeature
* 
* Get the next if-feature clause (if any)
*
* INPUTS:
*     iffeature == current iffeature struct
*
* RETURNS:
*     pointer to next if-feature struct
*     NULL if none available
*********************************************************************/
const ncx_iffeature_t *
    obj_get_next_iffeature (const ncx_iffeature_t  *iffeature)
{

#ifdef DEBUG
    if (!iffeature) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (const ncx_iffeature_t *)dlq_nextEntry(iffeature);

}  /* obj_get_next_iffeature */


/********************************************************************
* FUNCTION obj_is_enabled
* 
* Check any if-feature statement that may
* cause the specified object to be invisible
*
* INPUTS:
*    obj == obj_template_t to check

* RETURNS:
*    TRUE if object is enabled
*    FALSE if any if-features are present and FALSE
*********************************************************************/
boolean
    obj_is_enabled (const obj_template_t *obj)
{
    const ncx_iffeature_t   *iffeature;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    for (iffeature = obj_get_first_iffeature(obj);
	 iffeature != NULL;
	 iffeature = obj_get_next_iffeature(iffeature)) {

	if (!iffeature->feature ||
	    !ncx_feature_enabled(iffeature->feature)) {
	    return FALSE;
	}
    }

    return TRUE;

}  /* obj_is_enabled */


/********************************************************************
 * FUNCTION obj_is_single_instance
 * 
 * Check if the object is a single instance of if it
 * allows multiple instances; check all of the
 * ancestors if needed
 *
 * INPUTS:
 *    obj == object template to check
 *********************************************************************/
boolean
    obj_is_single_instance (obj_template_t *obj)
{
    ncx_iqual_t  iqual;

#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return TRUE;
    }
#endif

    while (obj != NULL) {
	iqual = obj_get_iqualval(obj);
	switch (iqual) {
	case NCX_IQUAL_ZMORE:
	case NCX_IQUAL_1MORE:
	    return FALSE;
	default:
	    /* don't bother checking the root
	     * and don't go past the root into
	     * the RPC parameters
	     */
	    obj = obj->parent;
	    if (obj && obj_is_root(obj)) {
		obj = NULL;
	    }
	}
    }
    return TRUE;

}  /* obj_is_single_instance */


/********************************************************************
 * FUNCTION obj_is_short_case
 * 
 * Check if the object is a short case statement
 *
 * INPUTS:
 *    obj == object template to check
 *********************************************************************/
boolean
    obj_is_short_case (obj_template_t *obj)
{
    const obj_case_t   *cas;

#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return TRUE;
    }
#endif

    if (obj->objtype != OBJ_TYP_CASE) {
        return FALSE;
    }

    cas = obj->def.cas;

    if (dlq_count(cas->datadefQ) != 1) {
        return FALSE;
    }

    if (obj->when && obj->when->exprstr) {
        return FALSE;
    }

    if (obj_get_first_iffeature(obj) != NULL) {
        return FALSE;
    }

    if (obj_get_status(obj) != NCX_STATUS_CURRENT) {
        return FALSE;
    }

    if (obj_get_description(obj) != NULL) {
        return FALSE;
    }

    if (obj_get_reference(obj) != NULL) {
        return FALSE;
    }

    if (dlq_count(obj_get_appinfoQ(obj)) > 0) {
        return FALSE;
    }

    return TRUE;


}  /* obj_is_short_case */


/********************************************************************
 * FUNCTION obj_has_when_stmts
 * 
 * Check if any when-stmts apply to this object
 * Does not check if they are true, just any when-stmts present
 *
 * INPUTS:
 *    obj == object template to check
 *********************************************************************/
boolean
    obj_has_when_stmts (obj_template_t *obj)
{
#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    if (obj->when) {
        return TRUE;;
    }
    if (obj->augobj && obj->augobj->when) {
        return TRUE;
    }
    if (obj->usesobj && obj->usesobj->when) {
        return TRUE;
    }
    return FALSE;

}  /* obj_has_when_stmts */


/********************************************************************
 * FUNCTION obj_sort_children
 * 
 * Check all the child nodes of the specified object
 * and rearrange them into alphabetical order,
 * based on the element local-name.
 *
 * ONLY SAFE TO USE FOR ncx:cli CONTAINERS
 * YANG DATA CONTENT ORDER NEEDS TO BE PRESERVED
 *
 * INPUTS:
 *    obj == object template to reorder
 *********************************************************************/
void
    obj_sort_children (obj_template_t *obj)
{
    obj_template_t    *newchild, *curchild;
    dlq_hdr_t         *datadefQ, sortQ;
    boolean            done;
    int                retval;

#ifdef DEBUG
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    datadefQ = obj_get_datadefQ(obj);
    if (datadefQ == NULL) {
        return;
    } 

    dlq_createSQue(&sortQ);
    newchild = (obj_template_t *)dlq_deque(datadefQ);
    while (newchild != NULL) {
        if (!obj_has_name(newchild)) {
            dlq_enque(newchild, &sortQ);
        } else {
            obj_sort_children(newchild);

            done = FALSE;
            for (curchild = (obj_template_t *)
                     dlq_firstEntry(&sortQ);
                 curchild != NULL && !done;
                 curchild = (obj_template_t *)
                     dlq_nextEntry(curchild)) {
            
                if (!obj_has_name(curchild)) {
                    continue;
                }

                retval = xml_strcmp(obj_get_name(newchild),
                                    obj_get_name(curchild));
                if (retval == 0) {        
                   if (obj_get_nsid(newchild) 
                        < obj_get_nsid(curchild)) {
                        dlq_insertAhead(newchild, curchild);
                    } else {
                        dlq_insertAfter(newchild, curchild);                    
                    }
                   done = TRUE;
                } else if (retval < 0) {
                    dlq_insertAhead(newchild, curchild);
                    done = TRUE;
                }
            }
            
            if (!done) {
                dlq_enque(newchild, &sortQ);
            }
        }
        newchild = (obj_template_t *)dlq_deque(datadefQ);
    }

    dlq_block_enque(&sortQ, datadefQ);

}  /* obj_sort_children */


/* END obj.c */
