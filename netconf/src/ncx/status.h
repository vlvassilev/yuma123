#ifndef _H_status
#define _H_status
/*  FILE: status.h
*********************************************************************
*                                                                    *
*                         P U R P O S E                              *
*                                                                    *
*********************************************************************

    global error status code enumerations

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
03-jan-92    abb      move statusT from global.h to here 
18-apr-96    abb      adapted for gr8cgi project
*/


/*********************************************************************
*                                                                    *
*                          C O N S T A N T S                         *
*                                                                    *
*********************************************************************/


/*
 * error code definitions--be sure to add any err code
 * definitions to the copy in err.c to register the errcode name,
 * the error severity, and the error string 
 */  

/* group the error messages by severity */
#define ERR_INT_BASE        0        /* really 2, hold 'NO_ERR' here */
#define ERR_FIRST_INT        2
#define ERR_SYS_BASE        100
#define ERR_USR_BASE        200
#define ERR_WARN_BASE        400
#define ERR_INFO_BASE   500

/* for backward compatability */
#define statusT         status_t

#define SET_ERROR(E) set_error(__FILE__, __LINE__, E, 0)

#define SET_DB_ERROR(E) set_error(__FILE__, __LINE__, E, 1)


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

typedef enum errtyp_t_
{
    ERR_TYP_NONE,
    ERR_TYP_INTERNAL,
    ERR_TYP_SYSTEM,
    ERR_TYP_USER,
    ERR_TYP_WARN,
    ERR_TYP_INFO
} errtyp_t;

/* global error return code */
typedef enum status_t_
{
    NO_ERR,                             /* 000 */
    ERR_END_OF_FILE,                    /* 001 */
        
    /* internal errors start at 2 */
    ERR_INTERNAL_PTR=ERR_FIRST_INT,     /* 002 */
    ERR_INTERNAL_MEM,                   /* 003 */
    ERR_INTERNAL_VAL,                   /* 004 */
    ERR_INTERNAL_BUFF,                  /* 005 */
    ERR_INTERNAL_QDEL,                  /* 006 */
    ERR_INTERNAL_INIT_SEQ,              /* 007 */
    ERR_QNODE_NOT_HDR,                  /* 008 */
    ERR_QNODE_NOT_DATA,                 /* 009 */
    ERR_BAD_QLINK,                      /* 010 */
    ERR_Q_ALREADY,                      /* 011 */
    ERR_TOO_MANY_ENTRIES,               /* 012 */
    ERR_XML2_FAILED,                    /* 013 */
    ERR_LAST_INT_ERR,                   /* 014 -- not really used */

    /* system errors start at 100 */
    ERR_FIL_OPEN=ERR_SYS_BASE,          /* 100 */
    ERR_FIL_READ,                       /* 101 */
    ERR_FIL_CLOSE,                      /* 102 */
    ERR_FIL_WRITE,                      /* 103 */
    ERR_FIL_CHDIR,                      /* 104 */
    ERR_FIL_STAT,                       /* 105 */
    ERR_BUFF_OVFL,                      /* 106 */
    ERR_FIL_DELETE,                     /* 107 */
    ERR_FIL_SETPOS,                     /* 108 */
    ERR_DB_CONNECT_FAILED,              /* 109 */
    ERR_DB_ENTRY_EXISTS,                /* 110 */
    ERR_DB_NOT_FOUND,                   /* 111 */
    ERR_DB_QUERY_FAILED,                /* 112 */
    ERR_DB_DELETE_FAILED,               /* 113 */
    ERR_DB_WRONG_CKSUM,                 /* 114 */
    ERR_DB_WRONG_TAGTYPE,               /* 115 */
    ERR_DB_READ_FAILED,                 /* 116 */
    ERR_DB_WRITE_FAILED,                /* 117 */
    ERR_DB_INIT_FAILED,                 /* 118 */
    ERR_TR_BEEP_INIT,                   /* 119 */
    ERR_TR_BEEP_NC_INIT,                /* 120 */
    ERR_XML_READER_INTERNAL,            /* 121 */
    ERR_OPEN_DIR_FAILED,                /* 122 */
    ERR_READ_DIR_FAILED,                /* 123 */
    ERR_LAST_SYS_ERR,                   /* 124 -- not really used */

    /* user errors start at 200 */
    ERR_NO_CFGFILE=ERR_USR_BASE,        /* 200 */
    ERR_NO_SRCFILE,                     /* 201 */
    ERR_PARSPOST_RD_INPUT,              /* 202 */
    ERR_FIL_BAD_DRIVE,                  /* 203 */
    ERR_FIL_BAD_PATH,                   /* 204 */
    ERR_FIL_BAD_FILENAME,               /* 205 */
    ERR_DUP_VALPAIR,                    /* 206 */
    ERR_PAGE_NOT_HANDLED,               /* 207 */
    ERR_PAGE_ACCESS_DENIED,             /* 208 */
    ERR_MISSING_FORM_PARAMS,            /* 209 */
    ERR_FORM_STATE,                     /* 210 */
    ERR_DUP_NS,                         /* 211 */
    ERR_XML_READER_START_FAILED,        /* 212 */
    ERR_XML_READER_READ,                /* 213 */
    ERR_XML_READER_NODETYP,             /* 214 */
    ERR_XML_READER_NULLNAME,            /* 215 */
    ERR_XML_READER_NULLVAL,             /* 216 */
    ERR_XML_READER_WRONGNAME,           /* 217 */
    ERR_XML_READER_WRONGVAL,            /* 218 */
    ERR_XML_READER_WRONGEL,             /* 219 */
    ERR_XML_READER_EXTRANODES,          /* 220 */
    ERR_XML_READER_EOF,                 /* 221 */
    ERR_NCX_WRONG_LEN,                  /* 222 */
    ERR_NCX_ENTRY_EXISTS,               /* 223 */
    ERR_NCX_DUP_ENTRY,                  /* 224 */
    ERR_NCX_NOT_FOUND,                  /* 225 */
    ERR_NCX_MISSING_FILE,               /* 226 */
    ERR_NCX_UNKNOWN_PARM,               /* 227 */
    ERR_NCX_INVALID_NAME,               /* 228 */
    ERR_NCX_UNKNOWN_NS,                 /* 229 */
    ERR_NCX_WRONG_NS,                   /* 230 */
    ERR_NCX_WRONG_TYPE,                 /* 231 */
    ERR_NCX_WRONG_VAL,                  /* 232 */
    ERR_NCX_MISSING_PARM,               /* 233 */
    ERR_NCX_EXTRA_PARM,                 /* 234 */
    ERR_NCX_EMPTY_VAL,                  /* 235 */
    ERR_NCX_MOD_NOT_FOUND,              /* 236 */
    ERR_NCX_LEN_EXCEEDED,               /* 237 */
    ERR_NCX_INVALID_TOKEN,              /* 238 */
    ERR_NCX_UNENDED_QSTRING,            /* 239 */
    ERR_NCX_READ_FAILED,                /* 240 */
    ERR_NCX_INVALID_NUM,                /* 241 */
    ERR_NCX_INVALID_HEXNUM,             /* 242 */
    ERR_NCX_INVALID_REALNUM,            /* 243 */
    ERR_NCX_EOF,                        /* 244 */
    ERR_NCX_WRONG_TKTYPE,               /* 245 */
    ERR_NCX_WRONG_TKVAL,                /* 246 */
    ERR_NCX_BUFF_SHORT,                 /* 247 */
    ERR_NCX_INVALID_RANGE,              /* 248 */
    ERR_NCX_OVERLAP_RANGE,              /* 249 */
    ERR_NCX_DEF_NOT_FOUND,              /* 250 */
    ERR_NCX_DEFSEG_NOT_FOUND,           /* 251 */
    ERR_NCX_TYPE_NOT_INDEX,             /* 252 */
    ERR_NCX_INDEX_TYPE_NOT_FOUND,       /* 253 */
    ERR_NCX_TYPE_NOT_MDATA,             /* 254 */
    ERR_NCX_MDATA_NOT_ALLOWED,          /* 255 */
    ERR_NCX_TOP_NOT_FOUND,              /* 256 */

    /* match netconf errors here */
    ERR_NCX_IN_USE,                      /* 257 */
    ERR_NCX_INVALID_VALUE,               /* 258 */
    ERR_NCX_TOO_BIG,                     /* 259 */
    ERR_NCX_MISSING_ATTRIBUTE,           /* 260 */
    ERR_NCX_BAD_ATTRIBUTE,               /* 261 */
    ERR_NCX_UNKNOWN_ATTRIBUTE,           /* 262 */
    ERR_NCX_MISSING_ELEMENT,             /* 263 */
    ERR_NCX_BAD_ELEMENT,                 /* 264 */
    ERR_NCX_UNKNOWN_ELEMENT,             /* 265 */
    ERR_NCX_UNKNOWN_NAMESPACE,           /* 266 */
    ERR_NCX_ACCESS_DENIED,               /* 267 */
    ERR_NCX_LOCK_DENIED,                 /* 268 */
    ERR_NCX_RESOURCE_DENIED,             /* 269 */
    ERR_NCX_ROLLBACK_FAILED,             /* 270 */
    ERR_NCX_DATA_EXISTS,                 /* 271 */
    ERR_NCX_DATA_MISSING,                /* 272 */
    ERR_NCX_OPERATION_NOT_SUPPORTED,     /* 273 */
    ERR_NCX_OPERATION_FAILED,            /* 274 */
    ERR_NCX_PARTIAL_OPERATION,           /* 275 */

    /* netconf error extensions */
    ERR_NCX_WRONG_NAMESPACE,            /* 276 */
    ERR_NCX_WRONG_NODEDEPTH,            /* 277 */
    ERR_NCX_WRONG_OWNER,                /* 278 */
    ERR_NCX_WRONG_ELEMENT,              /* 279 */
    ERR_NCX_WRONG_ORDER,                /* 280 */
    ERR_NCX_EXTRA_NODE,                 /* 281 */
    ERR_NCX_WRONG_NODETYP,              /* 282 */
    ERR_NCX_WRONG_NODETYP_SIM,          /* 283 */
    ERR_NCX_WRONG_NODETYP_CPX,          /* 284 */
    ERR_NCX_WRONG_DATATYP,              /* 285 */
    ERR_NCX_WRONG_DATAVAL,              /* 286 */
    ERR_NCX_NUMLEN_TOOBIG,              /* 287 */
    ERR_NCX_NOT_IN_RANGE,               /* 288 */
    ERR_NCX_WRONG_NUMTYP,               /* 289 */
    ERR_NCX_EXTRA_ENUMCH,               /* 290 */
    ERR_NCX_VAL_NOTINSET,               /* 291 */
    ERR_NCX_EXTRA_LISTSTR,              /* 292 */
    ERR_NCX_UNKNOWN_PSD,                /* 293 */
    ERR_NCX_EXTRA_PARMINST,             /* 294 */
    ERR_NCX_EXTRA_CHOICE,               /* 295 */
    ERR_NCX_MISSING_CHOICE,             /* 296 */
    ERR_NCX_CFG_STATE,                  /* 297 */
    ERR_NCX_UNKNOWN_APP,                /* 298 */
    ERR_NCX_UNKNOWN_TYPE,               /* 299 */
    ERR_NCX_NO_ACCESS_ACL,              /* 300 */
    ERR_NCX_NO_ACCESS_LOCK,             /* 301 */
    ERR_NCX_NO_ACCESS_STATE,            /* 302 */
    ERR_NCX_NO_ACCESS_MAX,              /* 303 */
    ERR_NCX_WRONG_INDEX_TYPE,           /* 304 */
    ERR_NCX_WRONG_INSTANCE_TYPE,        /* 305 */
    ERR_NCX_MISSING_INDEX,              /* 306 */
    ERR_NCX_CFG_NOT_FOUND,              /* 307 */
    ERR_NCX_EXTRA_ATTR,                 /* 308 */
    ERR_NCX_MISSING_ATTR,               /* 309 */
    ERR_NCX_MISSING_VAL_INST,           /* 310 */
    ERR_NCX_EXTRA_VAL_INST,             /* 311 */
    ERR_NCX_NOT_WRITABLE,               /* 312 */
    ERR_NCX_INVALID_PATTERN,            /* 313 */
    ERR_NCX_WRONG_VERSION,              /* 314 */
    ERR_NCX_CONNECT_FAILED,             /* 315 */
    ERR_NCX_UNKNOWN_HOST,               /* 316 */
    ERR_NCX_SESSION_FAILED,             /* 317 */
    ERR_NCX_AUTH_FAILED,                /* 318 */
    ERR_NCX_UNENDED_COMMENT,            /* 319 */
    ERR_NCX_INVALID_CONCAT,             /* 320 */
    ERR_NCX_IMP_NOT_FOUND,              /* 321 */
    ERR_NCX_MISSING_TYPE,               /* 322 */
    ERR_NCX_RESTRICT_NOT_ALLOWED,       /* 323 */
    ERR_NCX_REFINE_NOT_ALLOWED,         /* 324 */
    ERR_NCX_DEF_LOOP,                   /* 325 */
    ERR_NCX_DEFCHOICE_NOT_OPTIONAL,     /* 326 */
    ERR_NCX_IMPORT_LOOP,                /* 327 */
    ERR_NCX_INCLUDE_LOOP,               /* 328 */
    ERR_NCX_EXP_MODULE,                 /* 329 */
    ERR_NCX_EXP_SUBMODULE,              /* 330 */
    ERR_NCX_PREFIX_NOT_FOUND,           /* 331 */
    ERR_NCX_IMPORT_ERRORS,              /* 332 */
    ERR_NCX_PATTERN_FAILED,             /* 333 */
    ERR_NCX_INVALID_TYPE_CHANGE,        /* 334 */
    ERR_NCX_MANDATORY_NOT_ALLOWED,      /* 335 */
    ERR_NCX_UNIQUE_TEST_FAILED,         /* 336 */  /* 12.1 */
    ERR_NCX_MAX_ELEMS_VIOLATION,        /* 337 */  /* 12.2 */
    ERR_NCX_MIN_ELEMS_VIOLATION,        /* 338 */  /* 12.3 */
    ERR_NCX_MUST_TEST_FAILED,           /* 339 */  /* 12.4 */
    ERR_NCX_DATA_REST_VIOLATION,        /* 340 */  /* not used */
    ERR_NCX_INSERT_MISSING_INSTANCE,    /* 341 */  /* 12.5 */
    ERR_NCX_NOT_CONFIG,                 /* 342 */
    ERR_NCX_INVALID_CONDITIONAL,        /* 343 */
    ERR_NCX_USING_OBSOLETE,             /* 344 */
    ERR_NCX_INVALID_AUGTARGET,          /* 345 */
    ERR_NCX_DUP_REFINE_STMT,            /* 346 */
    ERR_NCX_INVALID_DEV_STMT,           /* 347 */
    ERR_NCX_INVALID_XPATH_EXPR,         /* 348 */
    ERR_NCX_NO_XPATH_PARENT,            /* 349 */
    ERR_LAST_USR_ERR,                   /* 352 -- not really used */

    /* user warnings start at 400 */
    ERR_MAKFILE_DUP_SRC=ERR_WARN_BASE,  /* 400 */
    ERR_INC_NOT_FOUND,                  /* 401 */
    ERR_CMDLINE_VAL,                    /* 402 */
    ERR_CMDLINE_OPT,                    /* 403 */
    ERR_CMDLINE_OPT_UNKNOWN,            /* 404 */
    ERR_CMDLINE_SYNTAX,                 /* 405 */
    ERR_CMDLINE_VAL_REQUIRED,           /* 406 */
    ERR_FORM_INPUT,                     /* 407 */
    ERR_FORM_UNKNOWN,                   /* 408 */
    ERR_NCX_NO_INSTANCE,                /* 409 */
    ERR_NCX_SESSION_CLOSED,             /* 410 */
    ERR_NCX_DUP_IMPORT,                 /* 411 */
    ERR_NCX_INVALID_DUP_IMPORT,         /* 412 */
    ERR_NCX_TYPDEF_NOT_USED,            /* 413 */
    ERR_NCX_GRPDEF_NOT_USED,            /* 414 */
    ERR_NCX_IMPORT_NOT_USED,            /* 415 */
    ERR_NCX_DUP_UNIQUE_COMP,            /* 416 */
    ERR_NCX_STMT_IGNORED,               /* 417 */
    ERR_NCX_DUP_INCLUDE,                /* 418 */
    ERR_NCX_INCLUDE_NOT_USED,           /* 419 */
    ERR_NCX_DATE_PAST,                  /* 420 */
    ERR_NCX_DATE_FUTURE,                /* 421 */
    ERR_NCX_ENUM_VAL_ORDER,             /* 422 */
    ERR_NCX_BIT_POS_ORDER,              /* 423 */
    ERR_NCX_INVALID_STATUS,             /* 424 */
    ERR_NCX_DUP_AUGNODE,                /* 425 */
    ERR_NCX_DUP_IF_FEATURE,             /* 426 */
    ERR_NCX_USING_DEPRECATED,           /* 427 */
    ERR_NCX_MISSING_REFTARGET,          /* 428 */
    ERR_NCX_EMPTY_XPATH_RESULT,         /* 429 */
    ERR_NCX_NO_XPATH_CHILD,             /* 430 */
    ERR_NCX_NO_XPATH_NODES,             /* 431 */
    ERR_LAST_WARN,                      /* 432 -- not really used */

    /* system info return codes start at 500 */
    ERR_PARS_SECDONE=ERR_INFO_BASE,     /* 500 */
    ERR_NCX_SKIPPED,                    /* 501 */
    ERR_NCX_CANCELED,                   /* 502 */
    ERR_LAST_INFO                       /* 503 -- not really used */

} status_t;


extern status_t 
    set_error (const char *filename, int linenum, 
               status_t status, int sqlError);

extern void 
    print_errors (void);

extern void 
    clear_errors (void);

extern errtyp_t
    get_errtyp (status_t res);

extern const char *
    get_error_string (status_t res);

extern status_t
    errno_to_status (void);

#endif     /* _H_status */
