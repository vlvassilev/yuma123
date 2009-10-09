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
06-oct-09   abb       Change YANG_ env vars to YUMA_ 
*/

#include <xmlstring.h>

#ifndef _H_cap
#include "cap.h"
#endif

#ifndef _H_help
#include "help.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

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

/* path, file separator char */
#ifdef WINDOWS
#define NCXMOD_PSCHAR   '\\'
#else
#define NCXMOD_PSCHAR   '/'
#endif

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

#define NCXMOD_WITH_DEFAULTS (const xmlChar *)"ietf-with-defaults"

/* name of the NETCONF module containing NETCONF protocol definitions,
 * that is loaded by default during startup 
 */
#define NCXMOD_NETCONF        (const xmlChar *)"netconf"

#define NCXMOD_IETF_NETCONF   (const xmlChar *)"ietf-netconf"

#define NCXMOD_IETF_NETCONF_STATE (const xmlChar *)"ietf-netconf-state"

/* name of the NCX modules directory appended when YUMA_HOME or HOME
 * ENV vars used to construct NCX module filespec
 */
#define NCXMOD_DIR            (const xmlChar *)"modules"

/* name of the data direectory when YUMA_HOME or HOME
 * ENV vars used to construct an NCX filespec
 */
#define NCXMOD_DATA_DIR        (const xmlChar *)"data"

/* name of the scripts direectory when YUMA_HOME or HOME
 * ENV vars used to construct a NCX filespec
 */
#define NCXMOD_SCRIPT_DIR       (const xmlChar *)"scripts"

/* STD Environment Variable for user home directory */
#define NCXMOD_PWD          "PWD"

/* STD Environment Variable for user home directory */
#define USER_HOME           "HOME"

/* NCX Environment Variable for YANG/NCX user work home directory */
#define NCXMOD_HOME         "YUMA_HOME"

/* NCX Environment Variable for tools install directory
 * The default is /usr/share/yuma
 */
#define NCXMOD_INSTALL   "YUMA_INSTALL"

/* !! should import this from make !! */
#define NCXMOD_DEFAULT_INSTALL (const xmlChar *)"/usr/share/yuma"

/* NCX Environment Variable for MODULE search path */
#define NCXMOD_MODPATH      "YUMA_MODPATH"

/* NCX Environment Variable for DATA search path */
#define NCXMOD_DATAPATH      "YUMA_DATAPATH"

/* NCX Environment Variable for SCRIPTS search path */
#define NCXMOD_RUNPATH      "YUMA_RUNPATH"

/* per user yangcli internal data home */
#define NCXMOD_YUMA_DIR (const xmlChar *)"~/.yuma"

/* directory yangcli uses to store local per-session workdirs */
#define NCXMOD_YUMA_TEMPDIR (const xmlChar *)"~/.yuma/tmp"


/********************************************************************
*								    *
*			  T Y P E S                                 *
*								    *
*********************************************************************/

/* following 3 structs used for providing temporary 
 * work directories for yangcli sessions
 */

/* program-level temp dir control block */
typedef struct ncxmod_temp_progcb_t_ {
    dlq_hdr_t    qhdr;
    xmlChar     *source;
    dlq_hdr_t    temp_sescbQ;  /* Q of ncxmod_temp_sescb_t */
} ncxmod_temp_progcb_t;


/* session-level temp-dir control block */
typedef struct ncxmod_temp_sescb_t_ {
    dlq_hdr_t    qhdr;
    xmlChar     *source;
    uint32       sidnum;
    dlq_hdr_t    temp_filcbQ;  /* Q of ncxmod_temp_filcb_t */
} ncxmod_temp_sescb_t;


/* temporary file control block */
typedef struct ncxmod_temp_filcb_t_ {
    dlq_hdr_t       qhdr;
    xmlChar        *source;
    const xmlChar  *filename;  /* ptr into source */
} ncxmod_temp_filcb_t;


/* struct for storing YANG file search results
 * this is used by yangcli for schema auto-load
 * also for finding newest version, or all versions
 * within the module path
 */
typedef struct ncxmod_search_result_t_ {
    dlq_hdr_t      qhdr;
    xmlChar       *module;
    xmlChar       *revision;
    xmlChar       *namespace;
    xmlChar       *source;
    ncx_module_t  *mod;      /* back-ptr to found module if loaded */
    cap_rec_t     *cap;      /* back-ptr to source capabuility URI */
    status_t       res;
    boolean        capmatch;     /* set by yangcli */
} ncxmod_search_result_t;


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
    ncxmod_load_module (const xmlChar *modname,
			const xmlChar *revision,
                        dlq_hdr_t *savedevQ,
			ncx_module_t **retmod);

extern status_t 
    ncxmod_parse_module (const xmlChar *modname,
                         const xmlChar *revision,
                         dlq_hdr_t *savedevQ,
                         ncx_module_t **retmod);

extern ncxmod_search_result_t *
    ncxmod_find_module (const xmlChar *modname,
			const xmlChar *revision);

extern status_t 
    ncxmod_load_deviation (const xmlChar *deviname,
                           dlq_hdr_t *deviationQ);

extern status_t 
    ncxmod_load_imodule (const xmlChar *modname,
			 const xmlChar *revision,
			 yang_pcb_t *pcb,
			 yang_parsetype_t ptyp);

extern yang_pcb_t *
    ncxmod_load_module_xsd (const xmlChar *modname,
			    const xmlChar *revision,
			    boolean subtree_mode,
			    boolean with_submods,
			    boolean cookedmode,
                            dlq_hdr_t *savedevQ,
			    status_t  *res);


extern yang_pcb_t *
    ncxmod_load_module_diff (const xmlChar *modname,
			     const xmlChar *revision,
			     boolean subtree_mode,
			     boolean with_submods,
			     const xmlChar *modpath,
                             dlq_hdr_t  *savedevQ,
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
    ncxmod_make_data_filespec (const xmlChar *fname,
                               status_t *res);

extern xmlChar *
    ncxmod_make_data_filespec_from_src (const xmlChar *srcspec,
                                        const xmlChar *fname,
                                        status_t *res);

extern xmlChar *
    ncxmod_find_script_file (const xmlChar *fname,
			     status_t *res);

extern void
    ncxmod_set_yuma_home (const xmlChar *yumahome);

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

extern status_t
    ncxmod_list_data_files (help_mode_t helpmode,
                            boolean logstdout);

extern status_t
    ncxmod_list_script_files (help_mode_t helpmode,
                              boolean logstdout);

extern status_t
    ncxmod_list_yang_files (help_mode_t helpmode,
                            boolean logstdout);

extern status_t
    ncxmod_setup_tempdir (void);

extern ncxmod_temp_progcb_t *
    ncxmod_new_program_tempdir (status_t *res);

extern void
    ncxmod_free_program_tempdir (ncxmod_temp_progcb_t *progcb);

extern ncxmod_temp_sescb_t *
    ncxmod_new_session_tempdir (ncxmod_temp_progcb_t *progcb,
                                uint32 sidnum,
                                status_t *res);

extern void
    ncxmod_free_session_tempdir (ncxmod_temp_progcb_t *progcb,
                                 uint32 sidnum);

extern ncxmod_temp_filcb_t *
    ncxmod_new_session_tempfile (ncxmod_temp_sescb_t *sescb,
                                 const xmlChar *filename,
                                 status_t *res);

extern void
    ncxmod_free_session_tempfile (ncxmod_temp_filcb_t *filcb);


extern ncxmod_search_result_t *
    ncxmod_new_search_result (void);

extern ncxmod_search_result_t *
    ncxmod_new_search_result_ex (const ncx_module_t *mod);

extern void
    ncxmod_free_search_result (ncxmod_search_result_t *searchresult);

#endif	    /* _H_ncxmod */
