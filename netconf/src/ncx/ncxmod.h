#ifndef _H_ncxmod
#define _H_ncxmod

/*  FILE: ncxmod.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Module Load Manager
  
     - manages NCX module search path
     - loads NCX module files by module name

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
10-nov-05    abb      Begun
22-jan-08    abb      Add support for YANG import and include
                      Unlike NCX, forward references are allowed
                      so import/include loops have to be tracked
                      and prevented
16-feb-08   abb       Changed environment variables from NCX to YANG
                      Added YANG_INSTALL envvar as well.
22-jul-08   abb       Remove NCX support -- YANG only from now on
*/

#include <xmlstring.h>

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_yang
#include "yang.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

/* max user-configurable directories for NCX and YANG modules */
#define NCXMOD_MAX_SEARCHPATH   64

/* maximum abolute filespec */
#define NCXMOD_MAX_FSPEC_LEN 2047

/* path, file separator char : SET TO UNIX */
#define NCXMOD_PSCHAR   '/'

#define NCXMOD_HMCHAR   '~'

#define NCXMOD_ENVCHAR '$'

#define NCXMOD_DOTCHAR '.'

/* file extension for YANG Modules */
#define NCXMOD_YANG_SUFFIX        (const xmlChar *)"yang"


/* name of the NCX module containing agent boot parameters
 * loaded during startup 
 */
#define NCXMOD_NETCONFD   (const xmlChar *)"netconfd"

#define NCXMOD_NCX        (const xmlChar *)"ncx"

/* name of the NETCONF module containing NETCONF protocol definitions,
 * that is loaded by default during startup 
 */
#define NCXMOD_NETCONF        (const xmlChar *)"netconf"

/* name of the NCX modules directory appended when YANG_HOME or HOME
 * ENV vars used to construct NCX module filespec
 */
#define NCXMOD_DIR            (const xmlChar *)"modules"

/* name of the data direectory when YANG_HOME or HOME
 * ENV vars used to construct a NCX filespec
 */
#define NCXMOD_DATA_DIR        (const xmlChar *)"data"

/* name of the scripts direectory when YANG_HOME or HOME
 * ENV vars used to construct a NCX filespec
 */
#define NCXMOD_SCRIPT_DIR       (const xmlChar *)"scripts"

/* STD Environment Variable for user home directory */
#define NCXMOD_PWD          "PWD"

/* STD Environment Variable for user home directory */
#define USER_HOME           "HOME"

/* NCX Environment Variable for YANG/NCX user work home directory */
#define NCXMOD_HOME         "YANG_HOME"

/* NCX Environment Variable for tools install directory
 * The default is /usr/share/yang
 */
#define NCXMOD_INSTALL   "YANG_INSTALL"

/* !! should import this from make !! */
#define NCXMOD_DEFAULT_INSTALL ((const xmlChar *)"/usr/share/yang")

/* NCX Environment Variable for MODULE search path */
#define NCXMOD_MODPATH      "YANG_MODPATH"

/* NCX Environment Variable for DATA search path */
#define NCXMOD_DATAPATH      "YANG_DATAPATH"

/* NCX Environment Variable for SCRIPTS search path */
#define NCXMOD_RUNPATH      "YANG_RUNPATH"

/********************************************************************
*								    *
*			  T Y P E S                                 *
*								    *
*********************************************************************/


/* user function callback template to process a module
 * during a subtree traversal
 *
 * ncxmod_callback_fn_t
 * 
 * Used by the ncxmod_process_subtree function
 *
 * DESCRIPTION:
 *   Handle the current filename in the subtree traversal
 *   Parse the module and generate.
 *
 * INPUTS:
 *   fullspec == absolute or relative path spec, with filename and ext.
 *               this regular file exists, but has not been checked for
 *               read access of 
 *   cookie == opaque handle passed from start of callbacks
 *
 * RETURNS:
 *    status
 *
 *    Return fatal error to stop the traversal or NO_ERR to
 *    keep the traversal going.  Do not return any warning or
 *    recoverable error, just log and move on
 *********************************************************************/
typedef status_t (*ncxmod_callback_fn_t) (const char *fullspec,
					  void *cookie);


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern void
    ncxmod_init (void);

extern void
    ncxmod_cleanup (void);

extern status_t 
    ncxmod_load_module (const xmlChar *modname);

extern status_t 
    ncxmod_load_imodule (const xmlChar *modname,
			 yang_pcb_t *pcb,
			 yang_parsetype_t ptyp);

extern yang_pcb_t *
    ncxmod_load_module_xsd (const xmlChar *modname,
			    boolean subtree_mode,
			    boolean with_submods,
			    status_t  *res);



extern yang_pcb_t *
    ncxmod_load_module_diff (const xmlChar *modname,
			     boolean subtree_mode,
			     boolean with_submods,
			     const xmlChar *modpath,
			     status_t  *res);


#ifdef NOT_YET
extern status_t 
    ncxmod_unload_module (const xmlChar *modname);
#endif

extern xmlChar *
    ncxmod_find_data_file (const xmlChar *fname,
			   boolean generrors,
			   status_t *res);

extern xmlChar *
    ncxmod_find_script_file (const xmlChar *fname,
			     status_t *res);

extern void
    ncxmod_set_modpath (const xmlChar *modpath);

extern void
    ncxmod_set_datapath (const xmlChar *datapath);

extern void
    ncxmod_set_runpath (const xmlChar *runpath);

extern void
    ncxmod_set_subdirs (boolean usesubdirs);

extern status_t
    ncxmod_process_subtree (const char *startspec, 
			    ncxmod_callback_fn_t callback,
			    void *cookie);

extern boolean
    ncxmod_test_subdir (const xmlChar *dirspec);

extern const xmlChar *
    ncxmod_get_userhome (const xmlChar *user,
			 uint32 userlen);


extern const xmlChar *
    ncxmod_get_envvar (const xmlChar *name,
		       uint32 namelen);

extern void
    ncxmod_set_altpath (const xmlChar *altpath);

extern void
    ncxmod_clear_altpath (void);

#endif	    /* _H_ncxmod */
