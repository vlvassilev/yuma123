/*  FILE: ncxmod.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
10nov05      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>
#include <unistd.h>
#include <xmlstring.h>
#include <xmlreader.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_ncxmod
#include "ncxmod.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif

#ifndef _H_yang_parse
#include "yang_parse.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define NCXMOD_DEBUG 1
#endif




/* Enumeration of the basic value type classifications */
typedef enum ncxmod_mode_t_ {
    NCXMOD_MODE_NONE,
    NCXMOD_MODE_YANG,
    NCXMOD_MODE_FILEYANG
} ncxmod_mode_t;


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
static boolean ncxmod_init_done = FALSE;

static const xmlChar *ncxmod_env_home;

static const xmlChar *ncxmod_env_install;

static const xmlChar *ncxmod_env_userhome;

static const xmlChar *ncxmod_mod_path = NULL;

static const xmlChar *ncxmod_alt_path;

static const xmlChar *ncxmod_data_path;

static const xmlChar *ncxmod_run_path;

static boolean ncxmod_subdirs;


/********************************************************************
* FUNCTION prep_dirpath
*
*  Setup the directory path in the buffer
*  Leave it with a trailing path-sep-char so a
*  file name can be added to the buffer
*
* INPUTS:
*    buff == buffer to use for filespec construction
*    bufflen == length of buffer
*    path == first piece of path string (may be NULL)
*    path2 == optional 2nd piece of path string (may be NULL)
*    cnt == address of return count of bytes added to buffer
*
* OUTPUTS:
*    buff filled in with path and path2 if present
*    *cnt == number of bytes added to buff
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    prep_dirpath (xmlChar *buff, 
		  uint32 bufflen,
		  const xmlChar *path, 
		  const xmlChar *path2,
		  uint32 *cnt)
{
    xmlChar *str;

    uint32 pathlen, path2len, pathsep, path2sep, total;

    *cnt = 0;
    *buff = 0;

    if (!path) {
	return NO_ERR;
    }
	
    pathlen = xml_strlen(path);
    if (pathlen) {
	pathsep = (uint32)(path[pathlen-1] == NCXMOD_PSCHAR);
    } else {
	pathsep = 0;
    }

    if (path2) {
	path2len = xml_strlen(path2);
	path2sep = (uint32)(path2len && path2[path2len-1]==NCXMOD_PSCHAR);
    } else {
	path2len = 0;
	path2sep = 0;
    }

    total =  pathlen + path2len;
    if (!pathsep && path2 && (*path2 != NCXMOD_PSCHAR)) {
	total++;
    }
    if (path2 && path2len && !path2sep) {
	total++;
    }

    if (*path == NCXMOD_HMCHAR && path[1] == NCXMOD_PSCHAR) {
	if (!ncxmod_env_userhome) {
	    return ERR_FIL_BAD_FILENAME;
	} else {
	    total += (xml_strlen(ncxmod_env_userhome) - 1);
	}
    }

    if (total >= bufflen) {
	log_error("\nncxmod: Path spec too long error. Max: %d Got %u",
		  bufflen, total);
	return ERR_BUFF_OVFL;
    }
	    
    str = buff;

    if (*path == NCXMOD_HMCHAR && path[1] == NCXMOD_PSCHAR) {
	str += xml_strcpy(str, ncxmod_env_userhome);
	str += xml_strcpy(str, &path[1]);
    } else {
	str += xml_strcpy(str, path);
    }

    if (!pathsep && path2 && (*path2 != NCXMOD_PSCHAR)) {
	*str++ = NCXMOD_PSCHAR;
    }

    if (path2 && path2len) {
	str += xml_strcpy(str, path2);
	if (!path2sep) {
	    *str++ = NCXMOD_PSCHAR;
	}
    }

    *str = 0;
    *cnt = (uint32)(str-buff);
    return NO_ERR;

}  /* prep_dirpath */


/********************************************************************
* FUNCTION try_module
*
* For NCX and YANG Modules Only!!!
*
* Construct a filespec out of a path name and a module name
* and try to load the filespec as an NCX module
*
* INPUTS:
*    buff == buffer to use for filespec construction
*    bufflen == length of buffer
*    path == first piece of path string (may be NULL)
*    path2 == optional 2nd piece of path string (may be NULL)
*    modname == module name without file suffix (may be NULL)
*    mode == suffix mode
*           == NCXMOD_MODE_YANG if YANG module and suffix is .yang
*           == NCXMOD_MODE_FILENCX if NCX filespec
*           == NCXMOD_MODE_FILEYANG if YANG filespec
*    usebuff == use buffer as-is, unless modname is present
*    done == address of return done flag
*    pcb == YANG parser control block (NULL if not used)
*    ptyp == YANG parser source type (YANG_PT_TOP if NCX)
*
* OUTPUTS:
*    *done == TRUE if module loaded and status==NO_ERR
*             or status != NO_ERR and fatal error occurred
*             The YANG pcb will also be updated
*
*          == FALSE if file-not-found-error or some non-fatal
*             error so the search should continue if possible
*
*    buff contains the full filespec of the found file
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    try_module (xmlChar *buff, 
	      uint32 bufflen,
	      const xmlChar *path, 
	      const xmlChar *path2,
	      const xmlChar *modname,
	      ncxmod_mode_t mode,
	      boolean usebuff,
	      boolean *done,
	      yang_pcb_t *pcb,
	      yang_parsetype_t ptyp)
{
    xmlChar       *p;
    const xmlChar *suffix;
    uint32         total, modlen, pathlen;
    status_t       res;
    
    *done = FALSE;
    total = 0;

    switch (mode) {
    case NCXMOD_MODE_YANG:
	suffix = NCXMOD_YANG_SUFFIX;
	break;
    case NCXMOD_MODE_FILEYANG:
	suffix = EMPTY_STRING;
	break;
    default:
	*done = TRUE;
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
    
    if (usebuff) {
	if (modname) {
	    /* add module name to end of buffer, if no overflow */
	    pathlen = xml_strlen(buff);
	    total =  pathlen + xml_strlen(modname) 
		+ xml_strlen(suffix) + 1;
	    if (total >= bufflen) {
		*done = TRUE;
		log_error("\nncxmod: Filename too long error. Max: %d Got %u",
			 NCXMOD_MAX_FSPEC_LEN, total);
		return ERR_BUFF_OVFL;
	    }
	    
	    p = buff+pathlen;
	    p += xml_strcpy(p, modname);
	    if (*suffix) {
		*p++ = '.';
		xml_strcpy(p, suffix);
	    }
	}
    } else {
	res = prep_dirpath(buff, bufflen, path, path2, &total);
	if (res != NO_ERR) {
	    *done = TRUE;
	    return res;
	}

	if (modname) {
	    modlen = xml_strlen(modname);

	    /* construct an complete module filespec 
	     * <buff><modname>.<suffix>
	     */
	    pathlen = xml_strlen(suffix);
	    if (pathlen) {
		pathlen += (modlen + 1);
	    }
	    if (total+pathlen >= bufflen) {
		log_info("\nncxmod: Filename too long error. Max: %d Got %u",
			 NCXMOD_MAX_FSPEC_LEN, total);
		return ERR_BUFF_OVFL;
	    }

	    /* add module name and suffix */
	    p = &buff[total];
	    p += xml_strcpy(p, modname);
	    if (*suffix) {
		*p++ = '.';
		xml_strcpy(p, suffix);
	    }
	}
    }

    /* attempt to load this filespec as a YANG module */
    res = yang_parse_from_filespec(buff, pcb, ptyp);
    switch (res) {
    case ERR_XML_READER_START_FAILED:
    case ERR_NCX_MISSING_FILE:
	/* not an error, *done == FALSE */
	if (mode==NCXMOD_MODE_YANG) {
	    res = NO_ERR;
	}
	break;
    default:
	/* NO_ERR or some other error */
        *done = TRUE;
    }

    return res;

}  /* try_module */


/********************************************************************
* FUNCTION test_file
*
* Construct a filespec out of a path name and a file name
* and try to find the filespec as an data or script file
*
* INPUTS:
*    buff == buffer to use for filespec construction
*    bufflen == length of buffer
*    path == first piece of path string
*    path2 == optional 2nd piece of path string
*    filename == complete filename with or without suffix
* 
* RETURNS:
*    TRUE if file found okay
*    FALSE if file-not-found-error
*********************************************************************/
static boolean
    test_file (xmlChar *buff, 
	       uint32 bufflen,
	       const xmlChar *path, 
	       const xmlChar *path2,
	       const xmlChar *filename)
{
    xmlChar    *p;
    uint32      flen, total;
    int         ret;
    status_t    res;
    struct stat statbuf;

    res = prep_dirpath(buff, bufflen, path, path2, &total);
    if (res != NO_ERR) {
	return res;
    }

    flen = xml_strlen(filename);
    if (flen+total >= bufflen) {
	log_error("\nError: Filename too long error. Max: %d Got %u",
		 NCXMOD_MAX_FSPEC_LEN, flen+total);
	return ERR_BUFF_OVFL;
    }

    p = &buff[total];
    p += xml_strcpy(p, filename);

    ret = stat((const char *)buff, &statbuf);
    return (ret == 0 && S_ISREG(statbuf.st_mode)) ? TRUE : FALSE;

}  /* test_file */


/********************************************************************
* FUNCTION test_pathlist
*
* Check the filespec path string for the specified module
* and suffix.  This function does not load any module
* It just finds the specified file.
*
* Subdirs are not checked.  The ENV vars that specify
* directories to search needs to add an entry for each
* dir to search
*
* INPUTS:
*    pathstr == pathstring list to check
*    buff == buffer to use for filespec construction
*    bufflen == length of buffer
*    modname == module name without file suffix
*    modsuffix == file suffix (no dot) [MAY BE NULL]
*
* OUTPUTS:
*    buff contains the complete path to the found file if
*    the return value is TRUE.  Ignore buff contents otherwise
*
* RETURNS:
*    TRUE if file found okay
*    FALSE if file-not-found-error
*********************************************************************/
static boolean
    test_pathlist (const xmlChar *pathlist,
		   xmlChar *buff,
		   uint32 bufflen,
		   const xmlChar *modname,
		   const xmlChar *modsuffix)
{
    const xmlChar *str, *p;
    uint32         len, mlen, slen, dot;
    int            ret;
    struct stat    statbuf;

    mlen = xml_strlen(modname);
    slen = (modsuffix) ? xml_strlen(modsuffix) : 0;
    dot = (uint32)((slen) ? 1 : 0);

    /* go through the path list and check each string */
    str = pathlist;
    while (*str) {
	/* find end of path entry or EOS */
	p = str+1;
	while (*p && *p != ':') {
	    p++;
	}
	len = (uint32)(p-str);
	if (len >= bufflen) {
	    SET_ERROR(ERR_BUFF_OVFL);
	    return FALSE;
	}

	/* copy the next string into buff */
	xml_strncpy(buff, str, len);

	/* make sure string ends with path sep char */
	if (buff[len-1] != NCXMOD_PSCHAR) {
	    if (len+1 >= bufflen) {
		SET_ERROR(ERR_BUFF_OVFL);
		return FALSE;
	    } else {
		buff[len++] = NCXMOD_PSCHAR;
	    }
	}

	/* add the module name and suffix */
	if (len+mlen+dot+slen >= bufflen) {
	    SET_ERROR(ERR_BUFF_OVFL);
	    return FALSE;
	}

	xml_strcpy(&buff[len], modname);
	if (modsuffix) {
	    buff[len+mlen] = '.';
	    xml_strcpy(&buff[len+mlen+1], modsuffix);
	}
		
	/* check if the file exists and is readable */
	ret = stat((const char *)buff, &statbuf);
	if (ret == 0) {
	    /* match in buff */
	    if (S_ISREG(statbuf.st_mode)) {
		return TRUE;
	    } else {
		/* should really be an error */
		return FALSE;
	    }
	}
    
	/* setup the next path string to try */
	if (*p) {
	    str = p+1;   /* one past ':' char */
	} else {
	    str = p;        /* already at EOS */
	}
    }

    return FALSE;

}  /* test_pathlist */


/********************************************************************
* FUNCTION search_subdirs
*
* Search any subdirs for the specified filename,
* starting at the specified location,
*
* INPUTS:
*    buff == buffer to use for filespec construction
*            at the start it contains the path string to use;
*            new directory names will be added to this buffer
*            as the subdirs are searched, until the 'fname' file
*            is found or an error occurs
*    bufflen == size of buff in bytes
*    fname == filename (with extension) to find
*    matchmode == TRUE if searching for a partial filename match
*              == FALSE for full filename match 
*    done == address of recurse done flag
*
* OUTPUTS:
*   *done == TRUE if done processing
*            FALSE to keep going
* RETURNS:
*    NO_ERR if file found okay, full filespec in the 'buff' variable
*    OR some error if not found or buffer overflow
*********************************************************************/
static status_t
    search_subdirs (xmlChar *buff, 
		    uint32 bufflen,
		    const xmlChar *fname,
		    boolean matchmode,
		    boolean *done)
{
    DIR           *dp;
    struct dirent *ep;
    struct stat    statbuf;
    uint32         pathlen, fnamelen, dentlen;
    int            ret;
    boolean        dirdone;
    status_t       res;

    *done = FALSE;
    pathlen = xml_strlen(buff);
    fnamelen = xml_strlen(fname);

    res = NO_ERR;

    if (pathlen+fnamelen+2 >= bufflen) {
	*done = TRUE;
	return ERR_BUFF_OVFL;
    } 

    /* make sure path ends with a pathsep char */
    if (buff[pathlen-1] != NCXMOD_PSCHAR) {
	buff[pathlen++] = NCXMOD_PSCHAR;
	buff[pathlen] = 0;
    }

    /* first see if the requested file is in this directory
     * do not use the dirent loop because the recursive
     * algorithm could result in a file at a lower level
     * in the subtree matching, instead of a file higher up
     * in the tree
     *
     * This will happen anyway if matchmode == TRUE
     */
    if (matchmode) {
	buff[pathlen] = 0;
    } else {
	/* try to open the full filespec in this directory
	 * do not bother if this is matchmode, since the
	 * filename is intentionally truncated, and might
	 * match something unrelated by mistake
	 */
	xml_strcpy(&buff[pathlen], fname);
	ret = stat((const char *)buff, &statbuf);
	if (ret == 0) {
	    *done = TRUE;
	    if (S_ISREG(statbuf.st_mode)) {
		res = NO_ERR;
	    } else {
		res = ERR_FIL_BAD_FILENAME;
	    }
	    return res;	
	} else {
	    buff[pathlen] = 0;
	}
    }

    /* try to open the buffer spec as a directory */
    dp = opendir((const char *)buff);
    if (!dp) {
	return NO_ERR;  /* not done yet */
    }

    dirdone = FALSE;
    while (!dirdone) {

	ep = readdir(dp);
	if (!ep) {
	    dirdone = TRUE;
	    continue;
	}

	/* this field may not be present on all POSIX systems
	 * according to the glibc 2.7 documentation!!
	 * only using dir file which works on linux. 
	 * !!!No support for symbolic links at this time!!!
	 *
	 * Always skip any directory or file that starts with
	 * the dot-char or is named CVS
	 */
	dentlen = xml_strlen((const xmlChar *)ep->d_name); 

	/* this dive-first behavior is not really what is desired
	 * but do not have a 'stat' function for partial filenames
	 * so just going through the directory block in order
	 */
	if (ep->d_type == DT_DIR) {
	    if (*ep->d_name != '.' && strcmp(ep->d_name, "CVS")) {
		if ((pathlen + dentlen) >= bufflen) {
		    res = ERR_BUFF_OVFL;
		    *done = TRUE;
		    dirdone = TRUE;
		} else {
		    xml_strcpy(&buff[pathlen], (const xmlChar *)ep->d_name);
		    res = search_subdirs(buff, bufflen, 
					 fname, matchmode, done);
		    if (*done) {
			dirdone = TRUE;
		    } else {
			/* erase the filename and keep trying */
			res = NO_ERR;
			buff[pathlen] = 0;
		    }
		}
	    }
	} else if (matchmode && (ep->d_type == DT_REG)) {
	    if (!xml_strncmp(fname, 
			     (const xmlChar *)ep->d_name,
			     fnamelen)) {
		/* filename is a partial match so check it out
		 * further to see if it is a match
		 * check if length matches foo.YYYY-MM-DD.yang
		 */
		if (dentlen == fnamelen + 16) {
		    /* check if the file extension is really .yang
		     * TBD: validate the date-string format
		     * even more TBD: search the entire dir
		     * for the highest valued date string
		     */
		    if (!strcmp(&ep->d_name[fnamelen+11], ".yang")) {
			*done = TRUE;
			if ((pathlen + dentlen) >= bufflen) {
			    res = ERR_BUFF_OVFL;
			} else {
			    res = NO_ERR;
			    xml_strcpy(&buff[pathlen], 
				       (const xmlChar *)ep->d_name);
			}
		    }
		}
	    }
	}
    }

    (void)closedir(dp);

    return res;

}  /* search_subdirs */


/********************************************************************
* FUNCTION add_failed
*
* Add a yang_node_t entry to the pcb->failedQ
*
* INPUTS:
*   modname == failed module name
*   revision == failed revision date (may be NULL)
*   pcb == parser control block
*   res == final result status for the failed module
*
* RETURNS:
*   status
*********************************************************************/
static status_t 
    add_failed (const xmlChar *modname,
		const xmlChar *revision,
		yang_pcb_t *pcb,
		status_t res)
{
    yang_node_t *node;

    node = yang_new_node();
    if (!node) {
	return ERR_INTERNAL_MEM;
    }

    node->failed = xml_strdup(modname);
    if (!node->failed) {
	yang_free_node(node);
	return ERR_INTERNAL_MEM;
    }

    if (revision) {
	node->failedrev = xml_strdup(revision);
	if (!node->failed) {
	    yang_free_node(node);
	    return ERR_INTERNAL_MEM;
	}
    }

    node->name = node->failed;
    node->revision = node->failedrev;
    node->res = res;
    dlq_enque(node, &pcb->failedQ);
    return NO_ERR;

} /* add_failed */


/********************************************************************
* FUNCTION check_module_path
*
*  Check the specified path for a YANG or NCX module file
*
* INPUTS:
*   path == starting path to check
*   buff == buffer to use for the filespec
*   bufflen == size of 'buff' in bytes
*   modname == module name to find (no file suffix)
*   revision == module revision date (may be NULL)
*   pcb == parser control block in progress
*   ptyp == parser source type
*   usepath == TRUE if path should be used directly
*              FALSE if the path should be appended with the 'modules' dir
*   matchmode == TRUE if partial filename match OK
*                FALSE for full filename match only
*   done == address of return done flag
*
* OUTPUTS:
*   *done == TRUE if file found or fatal error
*    file completely processed in try_module if file found
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_module_path (const xmlChar *path,
		       xmlChar *buff,
		       uint32 bufflen,
		       const xmlChar *modname,
		       const xmlChar *revision,
		       yang_pcb_t *pcb,
		       yang_parsetype_t ptyp,
		       boolean usepath,
		       boolean matchmode,
		       boolean *done)
{
    const xmlChar  *path2;
    xmlChar        *fnamebuff;
    uint32          total;
    status_t        res, res2;

    *done = FALSE;
    res = NO_ERR;
    res2 = NO_ERR;
    path2 = (usepath) ? NULL : NCXMOD_DIR;

    total = xml_strlen(path);
    if (total >= bufflen) {
	*done = TRUE;
	return ERR_BUFF_OVFL;
    }

    if (ncxmod_subdirs) {
	res = prep_dirpath(buff, bufflen, path, path2, &total);
	if (res != NO_ERR) {
	    *done = TRUE;
	    return res;
	}

	if (matchmode) {
	    fnamebuff = xml_strdup(modname);
	} else {
	    fnamebuff = yang_make_filename(modname, revision);
	}

	if (!fnamebuff) {
	    *done = TRUE;
	    return ERR_INTERNAL_MEM;
	}

	/* try YANG file */
	res = search_subdirs(buff, bufflen, 
			     fnamebuff, matchmode, done);
	if (*done) {
	    if (res == NO_ERR) {
		res = try_module(buff, bufflen, NULL, NULL,
				 NULL, NCXMOD_MODE_FILEYANG,
				 TRUE, done, pcb, ptyp);
		if (res != NO_ERR) {
		    res2 = add_failed(modname, revision,
				      pcb, res);
		}
	    }
	}
	m__free(fnamebuff);
	return (res2 != NO_ERR) ? res2 : res;
    }

    /* else subdir searches not allowed
     * first check for YANG file in the current path
     * then check for NCX file in the current path
     */
    res = try_module(buff, bufflen, path, path2, modname,
		     NCXMOD_MODE_YANG, FALSE, done, pcb, ptyp);
    if (*done) {
	if (res != NO_ERR) {
	    res2 = add_failed(modname, revision,
			      pcb, res);
	}
    }
    return (res2 != NO_ERR) ? res2 : res;

}  /* check_module_path */


/********************************************************************
* FUNCTION check_module_pathlist
*
*  Check a list of pathnames for the specified path of 
*  a YANG or NCX module file
*
*  Example:   path1:path2:path3
*
* INPUTS:
*   pathlist == formatted string containing list of path strings
*   buff == buffer to use for the filespec
*   bufflen == size of 'buff' in bytes
*   modname == module name to find (no file suffix)
*   revision == module revision date (may be NULL)
*   pcb == parser control block in progress
*   ptyp == parser source type
*   matchmode == TRUE if partial filename match OK
*                FALSE for full filename match only
*   done == address of return done flag
*
* OUTPUTS:
*   *done == TRUE if file found or fatal error
*    file completely processed in try_module if file found
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_module_pathlist (const xmlChar *pathlist,
			   xmlChar *buff,
			   uint32 bufflen,
			   const xmlChar *modname,
			   const xmlChar *revision,
			   yang_pcb_t *pcb,
			   yang_parsetype_t ptyp,
			   boolean matchmode,
			   boolean *done)
{
    const xmlChar  *str, *p;
    xmlChar        *pathbuff;
    uint32          pathbufflen, pathlen;
    status_t        res;

    pathbufflen = NCXMOD_MAX_FSPEC_LEN+1;
    pathbuff = m__getMem(pathbufflen);
    if (!pathbuff) {
	*done = TRUE;
	return ERR_INTERNAL_MEM;
    }

    /* go through the path list and check each string */
    str = pathlist;
    while (*str) {
	/* find end of path entry or EOS */
	p = str+1;
	while (*p && *p != ':') {
	    p++;
	}

	pathlen = (uint32)(p-str);
	if (pathlen >= pathbufflen) {
	    *done = TRUE;
	    m__free(pathbuff);
	    return ERR_BUFF_OVFL;
	}

	/* copy the next string into the path buffer */
	xml_strncpy(pathbuff, str, pathlen);
	res = check_module_path(pathbuff, buff, bufflen,
				modname, revision,
				pcb, ptyp, TRUE, matchmode, done);
	if (*done) {
	    m__free(pathbuff);
	    return res;
	}
    
	/* setup the next path string to try */
	if (*p) {
	    str = p+1;   /* one past ':' char */
	} else {
	    str = p;        /* already at EOS */
	}
    }

    m__free(pathbuff);
    *done = FALSE;
    return NO_ERR;

}  /* check_module_pathlist */


/********************************************************************
* FUNCTION load_module
*
* Determine the location of the specified module
* and then load it into the system, if not already loaded
*
* Module Search order:
*   1) filespec == try that only and exit
*   2) current directory
*   3) YANG_MODPATH environment var (or set by modpath CLI var)
*   4) HOME/modules directory
*   5) YANG_HOME/modules directory
*   6) YANG_INSTALL/modules directory OR
*   7) default install module location, which is '/usr/share/yang/modules'
*
* INPUTS:
*   modname == module name with no path prefix or file extension
*   revision == optional revision date of 'modname' to find
*   pcb == parser control block
*   ptyp == current parser source type
*   matchmode == TRUE if partial filename match OK
*                FALSE for full filename match only
*   retmod == address of return module (may be NULL)
*
* OUTPUTS:
*   if non-NULL:
*    *retmod == pointer to requested module version
*
* RETURNS:
*   status
*********************************************************************/
static status_t 
    load_module (const xmlChar *modname,
		 const xmlChar *revision,
		 yang_pcb_t *pcb,
		 yang_parsetype_t ptyp,
		 boolean matchmode,
		 ncx_module_t **retmod)
{
    const xmlChar  *str;
    xmlChar        *buff;
    ncx_module_t   *testmod;
    uint32          modlen, bufflen;
    status_t        res, res2;
    boolean         done, isfile;

#ifdef NCXMOD_DEBUG
    if (LOGDEBUG2) {
	log_debug2("\nAttempting to load module '%s'", modname);
	if (revision) {
	    log_debug2(" r:%s", revision);
	}
    }
#endif

    res = NO_ERR;
    res2 = NO_ERR;
    isfile = FALSE;
    bufflen = 0;
    done = FALSE;
    modlen = xml_strlen(modname);

    if (retmod) {
	*retmod = NULL;
    }

    /* check which form of input is present, module name or filespec */
    if ((*modname == '.') || (*modname == NCXMOD_PSCHAR)) {
	isfile = TRUE;
    } else {
	/* if a dir sep char is in the string it is automatically
	 * treated as an absolute filespec, even if it doesn't
	 * have a file suffix
	 */
	str = modname;
	while (*str && *str != NCXMOD_PSCHAR) {
	    str++;
	}
	if (*str) {
	    isfile = TRUE;
	}
    }

    /* find the start of the file name extension, expecting .yang  */
    str = &modname[modlen];
    while (str > modname && *str != '.') {
	str--;
    }

    /* try to find a .yang file suffix */
    if (*str == '.') {
	/* since the dot-char is allowed in YANG identifier names
	 * only treat this string with a dot in it as a file if
	 * it has a YANG file extension
	 */
	if (!xml_strcmp(++str, NCXMOD_YANG_SUFFIX)) {
	    isfile = TRUE;
	}
    }

    /* check valid module name pattern */
    if (!isfile && !ncx_valid_name(modname, modlen)) {
	log_error("\nError: Invalid module name (%s)", modname);
	res = add_failed(modname, revision, pcb, res);
	if (res != NO_ERR) {
	    return res;
	} else {
	    return ERR_NCX_INVALID_NAME;
	}
    }

    /* check if the module is already loaded (in the current ncx_modQ) */
    if (!isfile) {
	testmod = ncx_find_module(modname, revision);
	if (testmod) {
#ifdef NCXMOD_DEBUG
	    if (LOGDEBUG2) {
		log_debug2("\nncxmod: module %s already loaded", 
			   modname);
	    }
#endif
	    if (!pcb->top) {
		pcb->top = testmod;
	    }
	    if (retmod) {
		*retmod = testmod;
	    }
	    return testmod->status;
	}
    }

    /* get a temp buffer to construct filespacs */
    bufflen = NCXMOD_MAX_FSPEC_LEN+1;
    buff = m__getMem(bufflen);
    if (!buff) {
        return ERR_INTERNAL_MEM;
    } else {
	*buff = 0;
    }

    /* 1) if parameter is a filespec, then try it and exit
     *    if it does not work, instead of trying other directories
     */
    if (isfile) {
	res = try_module(buff, bufflen,  modname, NULL,
			 NULL, NCXMOD_MODE_FILEYANG,
			 FALSE, &done, pcb, ptyp);
	m__free(buff);
	if (res == ERR_NCX_MISSING_FILE) {
	    log_error("\nError: file not found (%s)", modname);
	} else if (res == NO_ERR) {
	    if (retmod) {
		*retmod = pcb->top;
	    }
	}
	return res;
    }

    /* 2) try alt_path variable if set; used by yangdiff */
    if (!done && ncxmod_alt_path) {
	res = check_module_path(ncxmod_alt_path, buff, bufflen,
				modname, revision,
				pcb, ptyp, FALSE, 
				matchmode, &done);
    }

    /* 3) try as module in current dir, YANG format  */
    if (!done) {
	res = try_module(buff, bufflen, NULL, NULL,
			 modname, NCXMOD_MODE_YANG,
			 FALSE, &done, pcb, ptyp);
    }

    /* 4) try YANG_MODPATH environment variable if set */
    if (!done && ncxmod_mod_path) {
	res = check_module_pathlist(ncxmod_mod_path, buff, bufflen,
				    modname, revision,
				    pcb, ptyp, 
				    matchmode, &done);
    }

    /* 5) HOME/modules directory */
    if (!done && ncxmod_env_userhome) {
	res = check_module_path(ncxmod_env_userhome, buff, bufflen,
				modname, revision,
				pcb, ptyp, FALSE, 
				matchmode, &done);
    }

    /* 6) YANG_HOME/modules directory */
    if (!done && ncxmod_env_home) {
	res = check_module_path(ncxmod_env_home, buff, bufflen,
				modname, revision,
				pcb, ptyp, FALSE, 
				matchmode, &done);
    }

    /* 7) YANG_INSTALL/modules directory or default install path
     *    If this envvar is set then the default install path will not
     *    be tried
     */
    if (!done) {
	if (ncxmod_env_install) {
	    res = check_module_path(ncxmod_env_install, buff, bufflen,
				    modname, revision,
				    pcb, ptyp, FALSE, 
				    matchmode, &done);
	} else {
	    res = check_module_path(NCXMOD_DEFAULT_INSTALL, 
				    buff, bufflen,
				    modname, revision,
				    pcb, ptyp, FALSE, 
				    matchmode, &done);
        }
    }

    res2 = NO_ERR;
    if (res != NO_ERR || !done) {
	res2 = add_failed(modname, revision, pcb, res);
    }

    if (res == NO_ERR) {
	res = res2;
    }

    m__free(buff);

    if (done) {
	if (res == NO_ERR && retmod) {
	    *retmod = pcb->top;
	}
	return res;
    } else {
	return (res == NO_ERR) ? ERR_NCX_MOD_NOT_FOUND : res;
    }

}  /* load_module */


/********************************************************************
* FUNCTION has_mod_ext
*
* Check if the filespec ends in '.yang' or '.ncx'
*
* INPUTS:
*    filespec == file spec string to check
*
* RETURNS:
*    TRUE if YANG or NCX file extension found
*       and non-zero filename\
*    FALSE otherwise
*********************************************************************/
static boolean
    has_mod_ext (const char *filespec)
{
    const char *p;

    p = filespec;
    while (*p) {
	p++;
    }

    while (p>filespec && (*p != '.')) {
	p--;
    }

    if (p==filespec) {
	return FALSE;
    }

    return (!strcmp(p+1, "yang")) 
	? TRUE : FALSE;
    
}  /* has_mod_ext */


/********************************************************************
* FUNCTION process_subtree
*
* Search the entire specified subtree, looking for YANG and
* NCX modules.  Invoke the callback function for each module
* file found
*
* INPUTS:
*    buff == working filespec buffer containing the dir spec 
*            to start with
*    bufflen == maxsize of buff, in bytes
*    callback == address of the ncxmod_callback_fn_t function
*         to use for this traveral
*    cookie == cookie to pass to each invocation of the callback
*
* RETURNS:
*    NO_ERR if file found okay, full filespec in the 'buff' variable
*    OR some error if not found or buffer overflow
*********************************************************************/
static status_t
    process_subtree (char *buff, 
		     uint32 bufflen,
		     ncxmod_callback_fn_t callback,
		     void *cookie)
{
    DIR           *dp;
    struct dirent *ep;
    uint32         pathlen;
    boolean        dirdone;
    status_t       res;


    res = NO_ERR;

    pathlen = xml_strlen((const xmlChar *)buff);
    if (!pathlen) {
	return NO_ERR;
    }

    /* make sure a min-length YANG file can be added (x.yang) */
    if (pathlen+8 >= bufflen) {
	return ERR_BUFF_OVFL;
    } 

    /* make sure dir-sep char is in place */
    if (buff[pathlen-1] != NCXMOD_PSCHAR) {
	buff[pathlen++] = NCXMOD_PSCHAR;
	buff[pathlen] = 0;
    }

    /* try to open the buffer spec as a directory */
    dp = opendir(buff);
    if (!dp) {
	return ERR_OPEN_DIR_FAILED;
    }

    dirdone = FALSE;
    while (!dirdone && res==NO_ERR) {

	ep = readdir(dp);
	if (!ep) {
	    dirdone = TRUE;
	    continue;
	}

	/* this field may not be present on all POSIX systems
	 * according to the glibc 2.7 documentation!!
	 * only using dir file which works on linux. 
	 * !!!No support for symbolic links at this time!!!
	 */
	switch (ep->d_type) {
	case DT_DIR:
	    if ((*ep->d_name != '.') && strcmp(ep->d_name, "CVS")) {
		if ((pathlen + 
		     xml_strlen((const xmlChar *)ep->d_name)) >=  bufflen) {
		    res = ERR_BUFF_OVFL;
		} else {
		    strcpy(&buff[pathlen], ep->d_name);
		    res = process_subtree(buff, bufflen, callback, cookie);
		    buff[pathlen] = 0;
		}
	    }
	    break;
	case DT_REG:
	    if ((*ep->d_name != '.') && has_mod_ext(ep->d_name)) {
		if ((pathlen + 
		     xml_strlen((const xmlChar *)ep->d_name)) >=  bufflen) {
		    res = ERR_BUFF_OVFL;
		} else {
		    strcpy(&buff[pathlen], ep->d_name);
		    res = (*callback)(buff, cookie);
		}
	    }
	    break;
	default:
	    ;
	}
    }

    (void)closedir(dp);

    return res;

}  /* process_subtree */


/********************************************************************
* FUNCTION try_load_module
*
* Try 1 to 3 forms of the YANG filespec to
* locate the specified module
*
* INPUTS:
*   pcb == parser control block to use
*   ptyp == parser mode type to use
*   modname == module name with no path prefix or file extension
*   revision == optional revision date of 'modname' to find
*   retmod == address of return module (may be NULL)
*
* OUTPUTS:
*   if non-NULL:
*    *retmod == pointer to requested module version
*
* RETURNS:
*   status
*********************************************************************/
static status_t 
    try_load_module (yang_pcb_t *pcb,
		     yang_parsetype_t ptyp,
		     const xmlChar *modname,
		     const xmlChar *revision,
		     ncx_module_t **retmod)
{
    ncx_module_t    *testmod;
    status_t         res;

#ifdef DEBUG
    if (!modname) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    testmod = NULL;

    res = load_module(modname, revision, 
		      pcb, ptyp, 
		      FALSE, &testmod);
    
    if (res == ERR_NCX_MOD_NOT_FOUND) {
	if (revision && *revision) {
	    /* try filenames without revision dates in them */
	    res = load_module(modname, NULL, 
			      pcb, ptyp, 
			      FALSE, &testmod);
	    if (res == NO_ERR && testmod) {
		if (!testmod->version) {
		    /* asked for a spcific version; 
		     * got a generic version instead
		     * rejected!; return error
		     */
		    res = ERR_NCX_WRONG_VERSION;
		} else if (yang_compare_revision_dates(revision,
						       testmod->version)) {
		    /* error should already be reported */
		    res = ERR_NCX_WRONG_VERSION;
		}
	    }
	} else {
	    /* already tried no revision date and it failed
	     * so try to match the module name in a
	     * filename with a revision date.
	     * The user does not care which version is found
	     * TBD: find the best version anyway
	     */
	    res = load_module(modname, NULL, 
			      pcb, ptyp, 
			      TRUE, &testmod);
	}
    }

    if (retmod) {
	*retmod = testmod;
    }

    return res;

}  /* try_load_module */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION ncxmod_init
* 

* Determine the location of the specified module
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
void
    ncxmod_init (void)
{
#ifdef DEBUG
    if (ncxmod_init_done) {
        SET_ERROR(ERR_INTERNAL_INIT_SEQ);
        return;
    }
#endif

    /* try to get the YANG_HOME environment variable */
    ncxmod_env_home = (const xmlChar *)getenv(NCXMOD_HOME);

    /* try to get the YANG_INSTALL environment variable */
    ncxmod_env_install = (const xmlChar *)getenv(NCXMOD_INSTALL);

    /* try to get the user HOME environment variable */
    ncxmod_env_userhome = (const xmlChar *)getenv(USER_HOME);

    /* try to get the module search path variable */
    ncxmod_mod_path = (const xmlChar *)getenv(NCXMOD_MODPATH);

    ncxmod_alt_path = NULL;

    /* try to get the data search path variable */
    ncxmod_data_path = (const xmlChar *)getenv(NCXMOD_DATAPATH);

    /* try to get the script search path variable */
    ncxmod_run_path = (const xmlChar *)getenv(NCXMOD_RUNPATH);

    ncxmod_subdirs = TRUE;

    ncxmod_init_done = TRUE;
    
}  /* ncxmod_init */


/********************************************************************
* FUNCTION ncxmod_cleanup
* 
* Cleanup the ncxmod module
*
*********************************************************************/
void
    ncxmod_cleanup (void)
{
#ifdef DEBUG
    if (!ncxmod_init_done) {
        SET_ERROR(ERR_INTERNAL_INIT_SEQ);
        return;
    }
#endif
     
    ncxmod_env_home = NULL;
    ncxmod_env_install = NULL;
    ncxmod_env_userhome = NULL;
    ncxmod_mod_path = NULL;
    ncxmod_data_path = NULL;
    ncxmod_run_path = NULL;

    ncxmod_init_done = FALSE;
    
}  /* ncxmod_cleanup */


/********************************************************************
* FUNCTION ncxmod_load_module
*
* Determine the location of the specified module
* and then load it into the system, if not already loaded
*
* Module Search order:
*
* 1) NCX_MODPATH environment var (or set by modpath CLI var)
* 2) current dir or absolute path
* 3) NCXHOME/modules directory
* 4) HOME/modules directory
*
* INPUTS:
*   modname == module name with no path prefix or file extension
*   revision == optional revision date of 'modname' to find
*   retmod == address of return module (may be NULL)
*
* OUTPUTS:
*   if non-NULL:
*    *retmod == pointer to requested module version
*
* RETURNS:
*   status
*********************************************************************/
status_t 
    ncxmod_load_module (const xmlChar *modname,
			const xmlChar *revision,
			ncx_module_t **retmod)
{
    yang_pcb_t      *pcb;
    status_t         res;

#ifdef DEBUG
    if (!modname) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    pcb = yang_new_pcb();
    if (!pcb) {
	res = ERR_INTERNAL_MEM;
    } else {
	pcb->revision = revision;
	res = try_load_module(pcb, YANG_PT_TOP,
			      modname, revision, retmod);
    }
    if (pcb) {
	yang_free_pcb(pcb);
    }
    return res;

}  /* ncxmod_load_module */


/********************************************************************
* FUNCTION ncxmod_load_imodule
*
* Determine the location of the specified module
* and then load it into the system, if not already loaded
*
* Called from an include or import or submodule
* Includes the YANG parser control block and new parser source type
*
* Module Search order:
*
* 1) NCX_MODPATH environment var (or set by modpath CLI var)
* 2) current dir or absolute path
* 3) NCXHOME/modules directory
* 4) HOME/modules directory
*
* INPUTS:
*   modname == module name with no path prefix or file extension
*   revision == optional revision date of 'modname' to find
*   pcb == YANG parser control block
*   ptyp == YANG parser source type
*
* RETURNS:
*   status
*********************************************************************/
status_t 
    ncxmod_load_imodule (const xmlChar *modname,
			 const xmlChar *revision,
			 yang_pcb_t *pcb,
			 yang_parsetype_t ptyp)
{
    yang_node_t     *node;
    const xmlChar   *savedrev;
    status_t         res;

#ifdef DEBUG
    if (!modname || !pcb) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* see if [sub]module already tried and failed */
    node = yang_find_node(&pcb->failedQ, modname, revision);
    if (node) {
	return node->res;
    }

    savedrev = pcb->revision;
    pcb->revision = revision;

    res = try_load_module(pcb, ptyp,
			  modname, revision, NULL);

    pcb->revision = savedrev;
    return res;

}  /* ncxmod_load_imodule */


/********************************************************************
* FUNCTION ncxmod_load_module_xsd
*
* Determine the location of the specified module
* and then load it into the system, if not already loaded
* Return the PCB instead of deleting it
*
* INPUTS:
*   modname == module name with no path prefix or file extension
*   revision == optional revision date of 'modname' to find
*   subtree_mode == TRUE if in a subtree loop
*                == FALSE if processing one module in yangdump
*   with_submods == TRUE if YANG_PT_TOP mode should skip submodules
*                == FALSE if top-level mode skip process sub-modules 
*   cookedmode == TRUE if producing cooked output
*                 FALSE if producing raw output
*   res == address of return status
*
* OUTPUTS:
*   *res == return status
*
* RETURNS:
*   pointer to malloced parser control block, or NULL of none
*********************************************************************/
yang_pcb_t *
    ncxmod_load_module_xsd (const xmlChar *modname,
			    const xmlChar *revision,
			    boolean subtree_mode,
			    boolean with_submods,
			    boolean cookedmode,
			    status_t *res)
{
    yang_pcb_t     *pcb;

#ifdef DEBUG
    if (!modname || !res) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    pcb = yang_new_pcb();
    if (!pcb) {
	*res = ERR_INTERNAL_MEM;
    } else {
	pcb->revision = revision;
	pcb->subtree_mode = subtree_mode;
	pcb->with_submods = with_submods;
	pcb->cookedmode = cookedmode;
	*res = try_load_module(pcb, YANG_PT_TOP,
			       modname, revision, NULL);
    }

    return pcb;

}  /* ncxmod_load_module_xsd */


/********************************************************************
* FUNCTION ncxmod_load_module_diff
*
* Determine the location of the specified module
* and then load it into the system, if not already loaded
* Return the PCB instead of deleting it
* !!Do not add definitions to the registry!!
*
* INPUTS:
*   modname == module name with no path prefix or file extension
*   revision == optional revision date of 'modname' to find
*   subtree_mode == TRUE if in a subtree loop
*                == FALSE if processing one module in yangdump
*   with_submods == TRUE if YANG_PT_TOP mode should skip submodules
*                == FALSE if top-level mode skip process sub-modules 
*   modpath == module path to override the modpath CLI var or
*              the YANG_MODPATH env var
*   res == address of return status
*
* OUTPUTS:
*   *res == return status
*
* RETURNS:
*   pointer to malloced parser control block, or NULL of none
*********************************************************************/
yang_pcb_t *
    ncxmod_load_module_diff (const xmlChar *modname,
			     const xmlChar *revision,
			     boolean subtree_mode,
			     boolean with_submods,
			     const xmlChar *modpath,
			     status_t *res)
{
    yang_pcb_t    *pcb;

#ifdef DEBUG
    if (!modname || !res) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    pcb = yang_new_pcb();
    if (!pcb) {
	*res = ERR_INTERNAL_MEM;
    } else {
	pcb->subtree_mode = subtree_mode;
	pcb->with_submods = with_submods;
	pcb->diffmode = TRUE;
	if (modpath) {
	    ncxmod_set_altpath(modpath);
	}
	*res = try_load_module(pcb, YANG_PT_TOP,
			       modname, revision, NULL);
    }

    return pcb;

}  /* ncxmod_load_module_diff */


/********************************************************************
* FUNCTION ncxmod_find_data_file
*
* Determine the location of the specified data file
*
* Search order:
*
* 1) current directory or absolute path
* 2) YANG_DATAPATH environment var (or set by datapath CLI var)
* 3) HOME/data directory
* 4) YANG_HOME/data directory
*
* INPUTS:
*   fname == file name with extension
*            if the first char is '.' or '/', then an absolute
*            path is assumed, and the search path will not be tries
*   generrors == TRUE if error message should be generated
*                FALSE if no error message
*   res == address of status result
*
* OUTPUTS:
*   *res == status 
*
* RETURNS:
*   pointer to the malloced and initialized string containing
*   the complete filespec or NULL if not found
*   It must be freed after use!!!
*********************************************************************/
xmlChar *
    ncxmod_find_data_file (const xmlChar *fname,
			   boolean generrors,
			   status_t *res)
{
    xmlChar  *buff;
    uint32    flen, bufflen;

#ifdef DEBUG
    if (!fname || !res) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    *res = NO_ERR;

#ifdef NCXMOD_DEBUG
    if (generrors) {
	if (LOGDEBUG2) {
	    log_debug2("\nNcxmod: Finding data file (%s)", 
		       fname);
	}
    }
#endif

    flen = xml_strlen(fname);
    if (!flen || flen>NCX_MAX_NLEN) {
        *res = ERR_NCX_WRONG_LEN;
	return NULL;
    }

    /* get a buffer to construct filespacs */
    bufflen = NCXMOD_MAX_FSPEC_LEN+1;
    buff = m__getMem(bufflen);
    if (!buff) {
	*res = ERR_INTERNAL_MEM;
        return NULL;
    }

    /* 1) current directory or absolute path */
    if (test_file(buff, bufflen, NULL, NULL, fname)) {
	return buff;
    }

    /* 2) try the NCX_DATAPATH environment variable */
    if (ncxmod_data_path) {
	if (test_pathlist(ncxmod_data_path, buff, bufflen, fname, NULL)) {
	    return buff;
	}
    }

    /* 3) HOME/data directory */
    if (ncxmod_env_userhome) {
        if (test_file(buff, bufflen, ncxmod_env_userhome, 
		      NCXMOD_DATA_DIR, fname)) {
            return buff;
        }
    }

    /* 4) YANG_HOME/data directory */
    if (ncxmod_env_home) {
        if (test_file(buff, bufflen, ncxmod_env_home,
		      NCXMOD_DATA_DIR, fname)) {
            return buff;
        }
    }

    if (generrors) {
	log_error("\nError: data file (%s) not found.", fname);
    }

    m__free(buff);
    *res = ERR_NCX_MOD_NOT_FOUND;
    return NULL;

}  /* ncxmod_find_data_file */


/********************************************************************
* FUNCTION ncxmod_find_script_file
*
* Determine the location of the specified script file
*
* Search order:
*
* 1) current directory or absolute path
* 2) YANG_RUNPATH environment var (or set by runpath CLI var)
* 3) HOME/scripts directory
* 4) YANG_HOME/scripts directory
* 5) YANG_INSTALL/scripts directory
*
* INPUTS:
*   fname == file name with extension
*            if the first char is '.' or '/', then an absolute
*            path is assumed, and the search path will not be tries
*   res == address of status result
*
* OUTPUTS:
*   *res == status 
*
* RETURNS:
*   pointer to the malloced and initialized string containing
*   the complete filespec or NULL if not found
*   It must be freed after use!!!
*********************************************************************/
xmlChar *
    ncxmod_find_script_file (const xmlChar *fname,
			     status_t *res)
{
    xmlChar  *buff;
    uint32    flen, bufflen;

#ifdef DEBUG
    if (!fname || !res) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    *res = NO_ERR;

#ifdef NCXMOD_DEBUG
    if (LOGDEBUG2) {
	log_debug2("\nNcxmod: Finding script file (%s)", 
		   fname);
    }
#endif

    flen = xml_strlen(fname);
    if (!flen || flen>NCX_MAX_NLEN) {
        *res = ERR_NCX_WRONG_LEN;
	return NULL;
    }

    /* get a buffer to construct filespacs */
    bufflen = NCXMOD_MAX_FSPEC_LEN+1;
    buff = m__getMem(bufflen);
    if (!buff) {
	*res = ERR_INTERNAL_MEM;
        return NULL;
    }

    /* 1) try 'fname' as a current directory or absolute path */
    if (test_file(buff, bufflen, NULL, NULL, fname)) {
	return buff;
    }

    /* look for the script file  'fname'
     * 2) check YANG_MODPATH env-var or modpath CLI param 
     */
    if (ncxmod_run_path) {
	if (test_pathlist(ncxmod_run_path, buff, bufflen, fname, NULL)) {
	    return buff;
	}
    }

    /* 3) try HOME/scripts/fname */
    if (ncxmod_env_userhome) {
        if (test_file(buff, bufflen, ncxmod_env_userhome, 
		      NCXMOD_SCRIPT_DIR, fname)) {
            return buff;
        }
    }

    /* 4) try YANG_HOME/scripts/fname */
    if (ncxmod_env_home) {
        if (test_file(buff, bufflen, ncxmod_env_home, 
		      NCXMOD_SCRIPT_DIR, fname)) {
            return buff;
        }
    }

    /* 5) YANG_INSTALL/scripts directory or default install path
     *    If this envvar is set then the default install path will not
     *    be tried
     */
    if (ncxmod_env_install) {
        if (test_file(buff, bufflen, ncxmod_env_install,
		      NCXMOD_SCRIPT_DIR, fname)) {
            return buff;
        }
    } else {
        if (test_file(buff, bufflen, NCXMOD_DEFAULT_INSTALL,
		      NCXMOD_SCRIPT_DIR, fname)) {
            return buff;
        }
    }

    log_info("\nError: script file (%s) not found.", fname);

    m__free(buff);
    *res = ERR_NCX_MOD_NOT_FOUND;
    return NULL;

}  /* ncxmod_find_script_file */


/********************************************************************
* FUNCTION ncxmod_set_modpath
* 
*   Override the NCX_MODPATH env var with the modpath CLI var
*
* THIS MAY GET SET DURING BOOTSTRAP SO SET_ERROR NOT CALLED !!!
*
*********************************************************************/
void
    ncxmod_set_modpath (const xmlChar *modpath)
{
#ifdef DEBUG
    if (!modpath) {
        return;
    }
#endif

    if (ncxmod_mod_path) {
	if (xml_strcmp(ncxmod_mod_path, modpath)) {
	    log_info("\nncxmod: Overriding NCX_MODPATH "
		     "env-var with CLI modpath");
	}
    }
    ncxmod_mod_path = modpath;
    
}  /* ncxmod_set_modpath */


/********************************************************************
* FUNCTION ncxmod_set_datapath
* 
*   Override the NCX_DATAPATH env var with the datapath CLI var
*
*********************************************************************/
void
    ncxmod_set_datapath (const xmlChar *datapath)
{
#ifdef DEBUG
    if (!datapath) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (ncxmod_data_path) {
	if (xml_strcmp(ncxmod_data_path, datapath)) {
	    log_info("\nncxmod: Overriding NCX_DATAPATH "
		     "env-var with CLI datapath");
	}
    }
    ncxmod_data_path = datapath;
    
}  /* ncxmod_set_datapath */


/********************************************************************
* FUNCTION ncxmod_set_runpath
* 
*   Override the NCX_RUNPATH env var with the runpath CLI var
*
*********************************************************************/
void
    ncxmod_set_runpath (const xmlChar *runpath)
{
#ifdef DEBUG
    if (!runpath) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif
    if (ncxmod_run_path) {
	if (xml_strcmp(ncxmod_run_path, runpath)) {
	    log_info("\nncxmod: Overriding NCX_RUNPATH "
		     "env-var with CLI runpath");
	}
    }
    ncxmod_run_path = runpath;
    
}  /* ncxmod_set_runpath */


/********************************************************************
* FUNCTION ncxmod_set_subdirs
* 
*   Set the subdirs flag to FALSE if the no-subdirs CLI param is set
*
* INPUTS:
*  usesubdirs == TRUE if subdirs searchs should be done
*             == FALSE if subdir searches should not be done
*********************************************************************/
void
    ncxmod_set_subdirs (boolean usesubdirs)
{
    ncxmod_subdirs = usesubdirs;
    
}  /* ncxmod_set_subdirs */


/********************************************************************
* FUNCTION ncxmod_process_subtree
*
* Search the entire specified subtree, looking for YANG and
* NCX modules.  Invoke the callback function for each module
* file found
*
* INPUTS:
*    startspec == absolute or relative pathspec to start
*            the search.  If this is not a valid pathname,
*            processing will exit immediately.
*    callback == address of the ncxmod_callback_fn_t function
*         to use for this traveral
*    cookie == cookie to pass to each invocation of the callback
*
* OUTPUTS:
*   *done == TRUE if done processing
*            FALSE to keep going
* RETURNS:
*    NO_ERR if file found okay, full filespec in the 'buff' variable
*    OR some error if not found or buffer overflow
*********************************************************************/
status_t
    ncxmod_process_subtree (const char *startspec, 
			    ncxmod_callback_fn_t callback,
			    void *cookie)
{
    char          *buff;
    DIR           *dp;
    uint32         bufflen;
    status_t       res;

#ifdef DEBUG
    if (!startspec || !callback) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (strlen(startspec) >= NCXMOD_MAX_FSPEC_LEN) {
	return ERR_BUFF_OVFL;
    }

    dp = opendir(startspec);
    if (!dp) {
	log_error("\nError: invalid pathspec '%s'", startspec);
	return ERR_NCX_INVALID_VALUE;
    } else {
	(void)closedir(dp);
    }

    bufflen = NCXMOD_MAX_FSPEC_LEN+1;
    buff = m__getMem(bufflen);
    if (!buff) {
	return ERR_INTERNAL_MEM;
    }

    strcpy(buff, startspec);
    res = process_subtree(buff, bufflen, callback, cookie);

    m__free(buff);
    return res;

}  /* ncxmod_process_subtree */


/********************************************************************
* FUNCTION ncxmod_test_subdir
*
* Check if the specified string is a directory
*
* INPUTS:
*    dirspec == string to check as a directory spec
*
* RETURNS:
*    TRUE if the string is a directory spec that this user
*       is allowed to open
*    FALSE otherwise
*********************************************************************/
boolean
    ncxmod_test_subdir (const xmlChar *dirspec)
{
    DIR           *dp;

    /* try to open the buffer spec as a directory */
    dp = opendir((const char *)dirspec);
    if (!dp) {
	return FALSE;
    } else {
	(void)closedir(dp);
	return TRUE;
    }
    /*NOTREACHED*/

}  /* ncxmod_test_subdir */


/********************************************************************
* FUNCTION ncxmod_get_userhome
*
* Get the user home dir from the passwd file
*
* INPUTS:
*    user == user name string (may not be zero-terminiated)
*    userlen == length of user
*
* RETURNS:
*    const pointer to the user home directory string
*********************************************************************/
const xmlChar *
    ncxmod_get_userhome (const xmlChar *user,
		  uint32 userlen)
{
    struct passwd  *pw;
    char            buff[NCX_MAX_USERNAME_LEN+1];

    /* only support user names up to N chars in length */
    if (userlen > NCX_MAX_USERNAME_LEN) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

    if (!user) {
	return (const xmlChar *)ncxmod_env_userhome;
    }

    strncpy(buff, (const char *)user, userlen);
    pw = getpwnam(buff);
    if (!pw) {
	return NULL;
    }

    return (const xmlChar *)pw->pw_dir;

}  /* ncxmod_get_userhome */


/********************************************************************
* FUNCTION ncxmod_get_envvar
*
* Get the specified shell environment variable
*
* INPUTS:
*    name == name of the environment variable (may not be zero-terminiated)
*    namelen == length of name string
*
* RETURNS:
*    const pointer to the specified environment variable value
*********************************************************************/
const xmlChar *
    ncxmod_get_envvar (const xmlChar *name,
		       uint32 namelen)
{
    char            buff[NCX_MAX_USERNAME_LEN+1];

#ifdef DEBUG
    if (!name) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    /* only support user names up to N chars in length */
    if (namelen > NCX_MAX_USERNAME_LEN) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

    strncpy(buff, (const char *)name, namelen);
    return (const xmlChar *)getenv(buff);

}  /* ncxmod_get_envvar */


/********************************************************************
* FUNCTION ncxmod_set_altpath
*
* Set the alternate path that should be used first (for yangdiff)
*
* INPUTS:
*    altpath == full path string to use
*               must be static 
*               a const back-pointer is kept, not a copy
*
*********************************************************************/
void
    ncxmod_set_altpath (const xmlChar *altpath)
{
#ifdef DEBUG
    if (!altpath) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    ncxmod_alt_path = altpath;

}  /* ncxmod_set_altpath */


/********************************************************************
* FUNCTION ncxmod_clear_altpath
*
* Clear the alternate path so none is used (for yangdiff)
*
*********************************************************************/
void
    ncxmod_clear_altpath (void)
{
    ncxmod_alt_path = NULL;

}  /* ncxmod_clear_altpath */


/* END file ncxmod.c */
