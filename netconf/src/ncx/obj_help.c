
/*  FILE: obj_help.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
17aug08      abb      begun; split out from obj.c

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

#ifndef _H_help
#include "help.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

/*
#ifndef _H_ncx
#include "ncx.h"
#endif
*/

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_obj_help
#include "obj_help.h"
#endif

#ifndef _H_status
#include "status.h"
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
* FUNCTION obj_dump_template
*
* Dump the contents of an obj_template_t struct for help text
*
* INPUTS:
*   obj == obj_template to dump help for
*   mode == requested help mode
*   nestlevel == number of levels from the top-level
*                that should be printed; 0 == all levels
*   indent == start indent count
*********************************************************************/
void
    obj_dump_template (const obj_template_t *obj,
		       help_mode_t mode,
		       uint32 nestlevel,
		       uint32 indent)
{
    const xmlChar    *val;
    char              numbuff[NCX_MAX_NUMLEN];
    uint32            count;

#ifdef DEBUG
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
    if (mode > HELP_MODE_FULL) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
#endif

    if (!obj_has_name(obj)) {
	return;
    }

    if (mode==HELP_MODE_NONE) {
	return;
    }

    if (nestlevel && (obj_get_level(obj) > nestlevel)) {
	return;
    }

    if (obj->objtype == OBJ_TYP_RPCIO) {
	help_write_lines(obj_get_name(obj), indent, TRUE);
	count = 0;
    } else if (obj->objtype == OBJ_TYP_CASE) {
	count = dlq_count(obj->def.cas->datadefQ);
    } else {
	count = 2;
    }
    if (count > 1) {
	help_write_lines(obj_get_typestr(obj), indent, TRUE);
	help_write_lines((const xmlChar *)" ", 0, FALSE);
	help_write_lines(obj_get_name(obj), 0, FALSE);
    }

    switch (obj->objtype) {
    case OBJ_TYP_RPC:
    case OBJ_TYP_RPCIO:
    case OBJ_TYP_CASE:
	break;
    default:
	if (obj->objtype != OBJ_TYP_CHOICE) {
	    help_write_lines((const xmlChar *)" [", 0, FALSE); 
	    help_write_lines((const xmlChar *)
			     obj_get_type_name(obj),
			     0, FALSE);
	    help_write_lines((const xmlChar *)"]", 0, FALSE); 
	}

	val = obj_get_default(obj);
	if (val) {
	    help_write_lines((const xmlChar *)" [", 0, FALSE); 
	    help_write_lines(val, 0, FALSE);
	    help_write_lines((const xmlChar *)"]", 0, FALSE); 
	}
    }

    val = obj_get_description(obj);
    if (val) {
	help_write_lines(val, indent+NCX_DEF_INDENT, TRUE); 
    }


    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	switch (mode) {
	case HELP_MODE_NORMAL:
	case HELP_MODE_FULL:
	    if (obj->def.container->presence) {
		help_write_lines((const xmlChar *)"presence: ", 
				 indent+NCX_DEF_INDENT, TRUE); 
		help_write_lines(obj->def.container->presence, 0, FALSE);
	    }
	    if (mode == HELP_MODE_FULL) {
		/*** add mustQ ***/;
	    }
	    break;
	default:
	    ;
	}
	obj_dump_datadefQ(obj->def.container->datadefQ, mode, 
			  nestlevel, indent+NCX_DEF_INDENT);
	break;
    case OBJ_TYP_LEAF:
	switch (mode) {
	case HELP_MODE_NORMAL:
	case HELP_MODE_FULL:
	    val = obj_get_units(obj);
	    if (val) {
		help_write_lines((const xmlChar *)"units: ", 
				 indent+NCX_DEF_INDENT, TRUE); 
		help_write_lines(val, 0, FALSE);
	    }
	    break;
	default:
	    ;
	}
	break;
    case OBJ_TYP_LEAF_LIST:
	switch (mode) {
	case HELP_MODE_NORMAL:
	case HELP_MODE_FULL:
	    val = obj_get_units(obj);
	    if (val) {
		help_write_lines((const xmlChar *)"units: ", 
				 indent+NCX_DEF_INDENT, TRUE); 
		help_write_lines(val, 0, FALSE);
	    }
	    if (!obj->def.leaflist->ordersys) {
		help_write_lines((const xmlChar *)"ordered-by: user", 
				 indent+NCX_DEF_INDENT, TRUE); 
	    }
	    if (obj->def.leaflist->minset) {
		help_write_lines((const xmlChar *)"min-elements: ", 
				 indent+NCX_DEF_INDENT, TRUE); 
		sprintf(numbuff, "%u", obj->def.leaflist->minelems);
		help_write_lines((const xmlChar *)numbuff, 0, FALSE);
	    }
	    if (obj->def.leaflist->maxset) {
		help_write_lines((const xmlChar *)"max-elements: ", 
				 indent+NCX_DEF_INDENT, TRUE); 
		sprintf(numbuff, "%u", obj->def.leaflist->maxelems);
		help_write_lines((const xmlChar *)numbuff, 0, FALSE);
	    }
	    break;
	default:
	    ;
	}
	break;
    case OBJ_TYP_CHOICE:
	count = dlq_count(obj->def.choic->caseQ);
	if (count) {
	    obj_dump_datadefQ(obj->def.choic->caseQ, mode, 
			      nestlevel, indent+NCX_DEF_INDENT);
	}
	break;
    case OBJ_TYP_CASE:
	count = dlq_count(obj->def.cas->datadefQ);
	if (count > 1) {
	    obj_dump_datadefQ(obj->def.cas->datadefQ, mode, 
			      nestlevel, indent+NCX_DEF_INDENT);
	} else if (count == 1) {
	    obj_dump_template(obj_first_child(obj), mode, 
			      nestlevel, indent);
	} /* else skip this case */
	break;
    case OBJ_TYP_LIST:
	switch (mode) {
	case HELP_MODE_NORMAL:
	case HELP_MODE_FULL:
	    if (obj->def.list->keystr) {
		help_write_lines((const xmlChar *)"key: ", 
				 indent+NCX_DEF_INDENT, TRUE); 
		help_write_lines(obj->def.list->keystr, 0, FALSE);
	    }
	    if (!obj->def.list->ordersys) {
		help_write_lines((const xmlChar *)"ordered-by: user", 
				 indent+NCX_DEF_INDENT, TRUE); 
	    }
	    if (obj->def.list->minset) {
		help_write_lines((const xmlChar *)"min-elements: ", 
				 indent+NCX_DEF_INDENT, TRUE); 
		sprintf(numbuff, "%u", obj->def.list->minelems);
		help_write_lines((const xmlChar *)numbuff, 0, FALSE);
	    }
	    if (obj->def.list->maxset) {
		help_write_lines((const xmlChar *)"max-elements: ", 
				 indent+NCX_DEF_INDENT, TRUE); 
		sprintf(numbuff, "%u", obj->def.list->maxelems);
		help_write_lines((const xmlChar *)numbuff, 0, FALSE);
	    }
	    break;
	default:
	    ;
	}
	obj_dump_datadefQ(obj->def.list->datadefQ, mode, 
			  nestlevel, indent+NCX_DEF_INDENT);
	break;
    case OBJ_TYP_RPC:
	if (mode != HELP_MODE_BRIEF &&
	    !dlq_empty(&obj->def.rpc->datadefQ)) {
	    obj_dump_datadefQ(&obj->def.rpc->datadefQ, mode, 
			      nestlevel, indent+NCX_DEF_INDENT);
	} else {
	    help_write_lines((const xmlChar *)"\n", 0, FALSE);
	}
	break;
    case OBJ_TYP_RPCIO:
	obj_dump_datadefQ(&obj->def.rpcio->datadefQ, mode, 
			  nestlevel, indent+NCX_DEF_INDENT);
	break;
    case OBJ_TYP_NOTIF:
	obj_dump_datadefQ(&obj->def.notif->datadefQ, mode, 
			  nestlevel, indent+NCX_DEF_INDENT);
    case OBJ_TYP_AUGMENT:
    case OBJ_TYP_USES:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }


}   /* obj_dump_template */


/********************************************************************
* FUNCTION obj_dump_datadefQ
*
* Dump the contents of a datadefQ for debugging
*
* INPUTS:
*   datadefQ == Q of obj_template to dump
*    full    == TRUE if a full report desired
*               FALSE if a partial report desired
*   nestlevel == number of levels from the top-level
*                that should be printed; 0 == all levels
*   indent == start indent count
*********************************************************************/
void
    obj_dump_datadefQ (const dlq_hdr_t *datadefQ,
		       help_mode_t mode,
		       uint32 nestlevel,
		       uint32 indent)
{

    const obj_template_t  *obj;

#ifdef DEBUG
    if (!datadefQ) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
    if (mode > HELP_MODE_FULL) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
#endif

    if (mode == HELP_MODE_NONE) {
	return;
    }

    for (obj = (const obj_template_t *)dlq_firstEntry(datadefQ);
	 obj != NULL;
	 obj = (const obj_template_t *)dlq_nextEntry(obj)) {

	if (!obj_has_name(obj)) {
	    continue;
	}

	obj_dump_template(obj, mode, nestlevel, indent);

	switch (obj->objtype) {
	case OBJ_TYP_RPCIO:
	case OBJ_TYP_CASE:
	    break;
	default:
	    help_write_lines((const xmlChar *)"\n", 0, FALSE);
	}
    }

}   /* obj_dump_datadefQ */


/* END obj_help.c */
