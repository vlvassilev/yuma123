/*
 * Copyright (c) 2009, Netconf Central, Inc.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
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

#ifndef _H_agt
#include "agt.h"
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


/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/

static val_value_t   *agt_caps = NULL;
static cap_list_t    *my_agt_caps = NULL;


/**************    E X T E R N A L   F U N C T I O N S **********/


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
*    defstyle == default with-defaults style for the entire agent
*
* RETURNS:
*    NO_ERR if all goes well
*********************************************************************/
status_t 
    agt_cap_set_caps (ncx_agttarg_t  agttarg,
                      ncx_agtstart_t agtstart,
                      const xmlChar *defstyle)
{
    const agt_profile_t   *agt_profile;
    val_value_t           *oldcaps, *newcaps;
    cap_list_t            *oldmycaps,*newmycaps;
    xmlns_id_t             nc_id, ncx_id;
    status_t               res;

    res = NO_ERR;
    newcaps = NULL;

    nc_id = xmlns_nc_id();
    ncx_id = xmlns_ncx_id();
    oldcaps = agt_caps;
    oldmycaps = my_agt_caps;

    agt_profile = agt_get_profile();

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

            if (res == NO_ERR) {
                res = cap_add_std(newmycaps, CAP_STDID_CONF_COMMIT);
                if (res == NO_ERR) {
                    res = cap_add_stdval(newcaps, CAP_STDID_CONF_COMMIT);
                }
            }
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

    if (res == NO_ERR) {
        if (agt_profile->agt_usevalidate) {
            /* set the validate capability */
            res = cap_add_std(newmycaps, CAP_STDID_VALIDATE);
            if (res == NO_ERR) {
                res = cap_add_stdval(newcaps, CAP_STDID_VALIDATE);
            }
        }
    }

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
    /* set the url capability */
    if (res == NO_ERR) {
        res = cap_add_url(newmycaps, (const xmlChar *)"file,sftp");
        if (res == NO_ERR) {
            ; /***/
        }
    }
#endif

    /* set the xpath capability */
    if (res == NO_ERR) {
        res = cap_add_std(newmycaps, CAP_STDID_XPATH);
        if (res == NO_ERR) {
            res = cap_add_stdval(newcaps, CAP_STDID_XPATH);
        }
    }

    /* set the notification capability */
    if (res == NO_ERR) {
        res = cap_add_std(newmycaps, CAP_STDID_NOTIFICATION);
        if (res == NO_ERR) {
            res = cap_add_stdval(newcaps, CAP_STDID_NOTIFICATION);
        }
    }

    /* set the interleave capability */
    if (res == NO_ERR) {
        res = cap_add_std(newmycaps, CAP_STDID_INTERLEAVE);
        if (res == NO_ERR) {
            res = cap_add_stdval(newcaps, CAP_STDID_INTERLEAVE);
        }
    }

#ifdef NOT_YET 
    /* set the partial-lock capability */
    if (res == NO_ERR) {
        res = cap_add_std(newmycaps, CAP_STDID_PARTIAL_LOCK);
        if (res == NO_ERR) {
            res = cap_add_stdval(newcaps, 
                                 CAP_STDID_PARTIAL_LOCK);
        }
    }
#endif

    /* set the with-defaults capability */
    if (res == NO_ERR) {
        res = cap_add_withdef(newmycaps, defstyle);
        if (res == NO_ERR) {
            res = cap_add_withdefval(newcaps, defstyle);
        }
    }

    /* set the netconf-monitoring capability */
    if (res == NO_ERR) {
        res = cap_add_std(newmycaps, 
                          CAP_STDID_NETCONF_MONITORING);
        if (res == NO_ERR) {
            res = cap_add_stdval(newcaps, 
                                 CAP_STDID_NETCONF_MONITORING);
        }
    }

    /* set the schema-retrieval capability */
    if (res == NO_ERR) {
        res = cap_add_std(newmycaps, 
                          CAP_STDID_SCHEMA_RETRIEVAL);
        if (res == NO_ERR) {
            res = cap_add_stdval(newcaps,
                                 CAP_STDID_SCHEMA_RETRIEVAL);
        }
    }

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
* INPUTS:
*   profile == agent profile control block to use
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    agt_cap_set_modules (agt_profile_t *profile)
{
    ncx_module_t          *mod;
    ncx_save_deviations_t *savedev;
    status_t               res;

    if (!agt_caps || !my_agt_caps) {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    res = NO_ERR;

    mod = ncx_get_first_module();

    /* add capability for each module loaded in ncxmod */
    while (mod && res == NO_ERR) {
        /* keep internal modules out of the capabilities */
        if (xml_strcmp(mod->name, NCX_EL_NETCONF) &&
            xml_strcmp(mod->name, NCX_EL_XSD)) {

            res = cap_add_modval(agt_caps, mod);
        }
        mod = (ncx_module_t *)dlq_nextEntry(mod);
    }

    /* add capability for each deviation module, not already
     * listed in the module capabilities so far
     */
    for (savedev = (ncx_save_deviations_t *)
             dlq_firstEntry(&profile->agt_savedevQ);
         savedev != NULL && res == NO_ERR;
         savedev = (ncx_save_deviations_t *)
             dlq_nextEntry(savedev)) {

        if (xml_strcmp(savedev->devmodule, NCX_EL_NETCONF) &&
            xml_strcmp(savedev->devmodule, NCX_EL_XSD)) {

            /* not a hard-wired internal module */
            mod = ncx_find_module(savedev->devmodule,
                                  savedev->devrevision);
            if (mod == NULL) {
                /* not already announced in the capabilities */
                res = cap_add_devmodval(agt_caps, savedev);
            }
        }
    }
         
    return res;

} /* agt_cap_set_modules */


/********************************************************************
* FUNCTION agt_cap_add_module
*
* Add a module at runtime, after the initial set has been set
* MUST call after agt_cap_set_caps
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    agt_cap_add_module (ncx_module_t *mod)
{
#ifdef DEBUG
    if (!mod) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (!agt_caps || !my_agt_caps) {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    return cap_add_modval(agt_caps, mod);

} /* agt_cap_add_module */


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
