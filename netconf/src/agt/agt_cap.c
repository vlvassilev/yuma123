/*  FILE: agt_cap.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
03feb06      abb      begun; split out from base/cap.c

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt_cap
#include "agt_cap.h"
#endif

#ifndef _H_cap
#include "cap.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxmod
#include "ncxmod.h"
#endif

#ifndef _H_ses
#include  "ses.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_typ
#include  "typ.h"
#endif

#ifndef _H_xml_val
#include  "xml_val.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/* #define AGT_CAP_DEBUG 1 */

#define AGT_CAP_SDISC_MODULE  (const xmlChar *)"schema-discovery"
#define AGT_CAP_SDISC_PARMSET (const xmlChar *)"schemaList"
#define AGT_CAP_SDISC_PARM    (const xmlChar *)"schema"

#define LEAF_SCHEMA_IDENTIFIER  (const xmlChar *)"identifier"
#define LEAF_SCHEMA_VERSION     (const xmlChar *)"version"
#define LEAF_SCHEMA_LOCATION    (const xmlChar *)"location"

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

static val_value_t   *agt_caps = NULL;
static cap_list_t    *my_agt_caps = NULL;

#if 0
/********************************************************************
* FUNCTION add_schema_parm
*
* Create and add a <schema> element for the <schemaList>
* which contains identifier, version, and location strings
* for a single module
*
* INPUTS:
*     ps == initialized schemaList parmset
*     modcap == module capability record to use for the info
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    add_schema_parm (val_value_t *valset,
		     const cap_rec_t *modcap)
{
    val_value_t    *parm, *val;
    xmlChar        *str;
    xmlns_id_t      nsid;
    status_t        res;

    chobj = obj_find_child(valset->obj, 
			   AGT_CAP_PREFIX,
			   AGT_CAP_SDISC_PARM);			   
    if (!chobj) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /*******  CONVERSION STOPPED HERE *************/

    /* create a new parm struct */
    parm = val_new_value();
    if (!parm) {
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(parm, chobj);

    /* get namespace for all nodes */
    nsid = obj_get_nsid(parm->obj);

    /* add identifier node */
    typdef = typ_find_child_typdef(LEAF_SCHEMA_IDENTIFIER,
				   parentdef);
    if (!typdef) {
	ps_free_parm(parm);
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
    val = val_make_simval(typdef, nsid,
			  LEAF_SCHEMA_IDENTIFIER,
			  modcap->cap_uri, &res);
    if (!val || res != NO_ERR) {
	ps_free_parm(parm);
	return res;
    }
    val_add_child(val, parm->val);

    /* add version node */
    typdef = typ_find_child_typdef(LEAF_SCHEMA_VERSION,
				   parentdef);
    if (!typdef) {
	ps_free_parm(parm);
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
    val = val_make_simval(typdef, nsid,
			  LEAF_SCHEMA_VERSION,
			  modcap->cap_ver, &res);
    if (!val || res != NO_ERR) {
	ps_free_parm(parm);
	return res;
    }
    val_add_child(val, parm->val);


    /* add location node */
    typdef = typ_find_child_typdef(LEAF_SCHEMA_LOCATION,
				   parentdef);
    if (!typdef) {
	ps_free_parm(parm);
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
    str = cap_make_mod_url(NC_OWNER, modcap);
    if (!str) {
	ps_free_parm(parm);
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
	
    val = val_make_simval(typdef, nsid,
			  LEAF_SCHEMA_LOCATION,
			  str, &res);
    m__free(str);
    if (!val || res != NO_ERR) {
	ps_free_parm(parm);
	return SET_ERROR(res);
    }
    val_add_child(val, parm->val);

    ps_add_parm(parm, ps, NCX_MERGE_LAST);
    return NO_ERR;

} /* add_schema_parm */
#endif



/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION agt_cap_init
*
* Initialize the NETCONF agent capabilities module
*
* INPUTS:
*    none
* RETURNS:
*    status
*********************************************************************/
status_t
    agt_cap_init (void)
{

    /*  return ncxmod_load_module(AGT_CAP_SDISC_MODULE); */
    return NO_ERR;
	
} /* agt_cap_init */


/********************************************************************
* FUNCTION agt_cap_cleanup
*
* Clean the NETCONF agent capabilities
*
* INPUTS:
*    none
* RETURNS:
*    none
*********************************************************************/
void 
    agt_cap_cleanup (void)
{
    if (agt_caps) {
	val_free_value(agt_caps);
	agt_caps = NULL;
    }

    if (my_agt_caps) {
	cap_clean_caplist(my_agt_caps);
	m__free(my_agt_caps);
	my_agt_caps = NULL;
    }

} /* agt_cap_cleanup */


/********************************************************************
* FUNCTION agt_cap_set_caps
*
* Initialize the NETCONF agent capabilities
*
* INPUTS:
*    agttarg == the target of edit-config for this agent
*    agtstart == the type of startup configuration for this agent
*
* RETURNS:
*    NO_ERR if all goes well
*********************************************************************/
status_t 
    agt_cap_set_caps (ncx_agttarg_t  agttarg,
		      ncx_agtstart_t agtstart)
{
    status_t  res;
    val_value_t *oldcaps, *newcaps;
    cap_list_t *oldmycaps,*newmycaps;
    xmlns_id_t  nc_id, ncx_id;

    res = NO_ERR;
    nc_id = xmlns_nc_id();
    ncx_id = xmlns_ncx_id();
    oldcaps = agt_caps;
    oldmycaps = my_agt_caps;

    /* get a new cap_list */
    newmycaps = cap_new_caplist();
    if (!newmycaps) {
	res = ERR_INTERNAL_MEM;
    }

    /* get a new val_value_t cap list for agent <hello> messages */
    if (res == NO_ERR) {
	newcaps = xml_val_new_struct(NCX_EL_CAPABILITIES, nc_id);
	if (!newcaps) {
	    res = ERR_INTERNAL_MEM;
	}
    }

    /* add capability for NETCONF version 1.0 support */
    if (res == NO_ERR) {
	res = cap_add_std(newmycaps, CAP_STDID_V1);
	if (res == NO_ERR) {
	    res = cap_add_stdval(newcaps, CAP_STDID_V1);
	}
    }

    if (res == NO_ERR) {
	/* set the capabilities based on the native target */
	switch (agttarg) {
	case NCX_AGT_TARG_RUNNING:
	    res = cap_add_std(newmycaps, CAP_STDID_WRITE_RUNNING);
	    if (res == NO_ERR) {
		res = cap_add_stdval(newcaps, CAP_STDID_WRITE_RUNNING);
	    }
	    break;
	case NCX_AGT_TARG_CANDIDATE:
	    res = cap_add_std(newmycaps, CAP_STDID_CANDIDATE);
	    if (res == NO_ERR) {
		res = cap_add_stdval(newcaps, CAP_STDID_CANDIDATE);
	    }

#ifdef NOT_YET 
	    res = cap_add_std(newmycaps, CAP_STDID_CONF_COMMIT);
	    if (res == NO_ERR) {
		res = cap_add_stdval(newcaps, CAP_STDID_COMMIT);
	    }
#endif
	    break;
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	    break;
	}
    }

    if (res == NO_ERR) {
	/* set the rollback-on-error capability */
	res = cap_add_std(newmycaps, CAP_STDID_ROLLBACK_ERR);
	if (res == NO_ERR) {
	    res = cap_add_stdval(newcaps, CAP_STDID_ROLLBACK_ERR);
	}
    }

#ifdef NOT_YET
    if (res == NO_ERR) {
	/* set the validate capability */
	res = cap_add_std(newmycaps, CAP_STDID_VALIDATE);
	if (res == NO_ERR) {
	    res = cap_add_stdval(newcaps, CAP_STDID_VALIDATE);
	}
    }
#endif

    /* check the startup type for distinct-startup capability */
    if (res == NO_ERR) {
	if (agtstart==NCX_AGT_START_DISTINCT) {
	    res = cap_add_std(newmycaps, CAP_STDID_STARTUP);
	    if (res == NO_ERR) {
		res = cap_add_stdval(newcaps, CAP_STDID_STARTUP);
	    }
	}
    }

#ifdef NOT_YET
    /* set the xpath capability */
    if (res == NO_ERR) {
	res = cap_add_std(&my_agt_caps, CAP_STDID_XPATH);
	if (res == NO_ERR) {
	    res = cap_add_stdval(newcaps, CAP_STDID_XPATH);
	}
    }
#endif

#ifdef NOT_YET
    /* set the url capability */
    if (res == NO_ERR) {
	res = cap_add_url(newmycaps, (const xmlChar *)"file,sftp");
	if (res == NO_ERR) {
	    ; /***/
	}
    }
#endif

    /* check the return value */
    if (res != NO_ERR) {
	/* toss the new, put back the old */
	cap_free_caplist(newmycaps);
	val_free_value(newcaps);
	my_agt_caps = oldmycaps;
	agt_caps = oldcaps;
    } else {
	/* toss the old, install the new */
	if (oldmycaps) {
	    cap_free_caplist(oldmycaps);
	}
	if (oldcaps) {
	    val_free_value(oldcaps);
	}
	my_agt_caps = newmycaps;
	agt_caps = newcaps;
    }	

    return res;

} /* agt_cap_set_caps */


/********************************************************************
* FUNCTION agt_cap_set_modules
*
* Initialize the NETCONF agent capabilities modules list
* MUST call after agt_cap_set_caps
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    agt_cap_set_modules (void)
{
    ncx_module_t  *mod;
    status_t       res;
    xmlns_id_t     nc_id, ncx_id;

    if (!agt_caps || !my_agt_caps) {
	return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    mod = ncx_get_first_module();
    if (!mod) {
	return NO_ERR;
    }

    res = NO_ERR;
    nc_id = xmlns_nc_id();
    ncx_id = xmlns_ncx_id();

    /* add capability for each module loaded in ncxmod */
    while (mod && res == NO_ERR) {
	res = cap_add_mod(my_agt_caps,
			  mod->name,
			  mod->version);
	if (res == NO_ERR) {
	    res = cap_add_modval(agt_caps,
				 mod->name,
				 mod->version);
	}
	if (res == NO_ERR) {
	    mod = (ncx_module_t *)dlq_nextEntry(mod);
	}
    }

    return res;

} /* agt_cap_set_modules */

#if 0
/********************************************************************
* FUNCTION agt_cap_set_modcaps_parmset
*
* Setup the schema-discovery 'modules' parmset
* MUST call after agt_cap_set_modules and after the
* <running> configuration is loaded
*
*********************************************************************/
void
    agt_cap_set_modcaps_parmset (void)
{
    const cap_rec_t  *modcap;
    val_value_t      *val;
    status_t          res;

    if (!agt_caps || !my_agt_caps) {
	SET_ERROR(ERR_INTERNAL_INIT_SEQ);
	return;
    }

    /* make sure there is at least one module to report */
    modcap = cap_first_modcap(my_agt_caps);
    if (!modcap) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    /* create a new parmset struct for schemaList */
    val = val_new_value();
    if (!val) {
	SET_ERROR(ERR_INTERNAL_MEM);
	return;
    }

    /* add schema parm for each module loaded in ncxmod */
    while (modcap) {

	/* add a schema child node for this module */
	res = add_schema_parm(ps, modcap);
	if (res != NO_ERR) {
	    ps_free_parmset(ps);
	    return;
	}

	/* setup next pass through the while loop */
	modcap = cap_next_modcap(modcap);
    }

    /* generate an instance ID for this parmset */
    res = ps_gen_parmset_instance_id(ps);
    if (res != NO_ERR) {
	ps_free_parmset(ps);
	SET_ERROR(res);
	return;
    }

    /* add the parmset to the netconf application node */
    res = cfg_add_parmset(cfg_get_config_id(NCX_CFGID_RUNNING),
			  ps, SES_NULL_SID);
    if (res != NO_ERR) {
	ps_free_parmset(ps);
	SET_ERROR(res);
    }

} /* agt_cap_set_modcaps_parmset */
#endif


/********************************************************************
* FUNCTION agt_cap_get_caps
*
* Get the NETCONF agent capabilities
*
* INPUTS:
*    none
* RETURNS:
*    pointer to the agent caps list
*********************************************************************/
cap_list_t * 
    agt_cap_get_caps (void)
{
    return my_agt_caps;

} /* agt_cap_get_caps */


/********************************************************************
* FUNCTION agt_cap_get_capsval
*
* Get the NETCONF agent capabilities ain val_value_t format
*
* INPUTS:
*    none
* RETURNS:
*    pointer to the agent caps list
*********************************************************************/
val_value_t * 
    agt_cap_get_capsval (void)
{
    return agt_caps;
} /* agt_cap_get_capsval */

/********************************************************************
* FUNCTION agt_cap_std_set
*
* Check if the STD capability is set for the agent
*
* INPUTS:
*    cap == ID of capability to check
*
* RETURNS:
*    TRUE is STD cap set, FALSE otherwise
*********************************************************************/
boolean
    agt_cap_std_set (cap_stdid_t cap)
{
    return cap_std_set(my_agt_caps, cap);

} /* agt_cap_std_set */


/* END file agt_cap.c */
