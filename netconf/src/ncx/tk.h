#ifndef _H_tk
#define _H_tk

/*  FILE: tk.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Module Compact Syntax Token Handler

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
12-nov-05    abb      Begun

*/

#include <xmlstring.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

/* maximum line size allowed in an NCX module */
#define TK_BUFF_SIZE                0xffff


/* macros for quick processing of token chains
 * All these macros take 1 parameter 
 * INPUTS:
 *   T == pointer to the tk_chain_t in progress
 */

/* advance the current token pointer */
#define TK_ADV(T)  \
    (((T)->cur = (tk_token_t *)dlq_nextEntry((T)->cur)) \
     ? NO_ERR : ERR_NCX_EOF)

/* back-up the current token pointer */
#define TK_BKUP(T) \
    if (!  ((T)->cur = (tk_token_t *)dlq_prevEntry((T)->cur)))  \
        (T)->cur = (tk_token_t *)&((T)->tkQ)

/* return the current token */
#define TK_CUR(T) ((T)->cur)

/* return the current token type */
#define TK_CUR_TYP(T) ((T)->cur->typ)

/* return the current token value */
#define TK_CUR_VAL(T) ((T)->cur->val)

/* return the current token value length */
#define TK_CUR_LEN(T) ((T)->cur->len)

/* return the current token module qualifier value */
#define TK_CUR_MOD(T) ((T)->cur->mod)

/* return the current token module qualifier value length */
#define TK_CUR_MODLEN(T) ((T)->cur->modlen)

/* return TRUE if the current token type is a string */
#define TK_CUR_STR(T) ((T)->cur->typ >= TK_TT_STRING && \
		       (T)->cur->typ <= TK_TT_SQSTRING)

/* return TRUE if the specified token type is a string */
#define TK_TYP_STR(T) ((T) >= TK_TT_STRING && (T) <= TK_TT_SQSTRING)

/* return TRUE if the cur tk type allows a non-whitespace string */
#define TK_CUR_NOWSTR(T) ((T)->cur->typ >= TK_TT_STRING && \
                          (T)->cur->typ <= TK_TT_SQSTRING)

/* return TRUE if the cu token type allows whitespace string */
#define TK_CUR_WSTR(T) ((T)->cur->typ==TK_TT_STRING || \
                        (T)->cur->typ==TK_TT_SSTRING ||  \
			(T)->cur->typ==TK_TT_TSTRING ||	 \
			(T)->cur->typ==TK_TT_QSTRING || \
			(T)->cur->typ==TK_TT_SQSTRING)

/* return TRUE if the current token type is a number */
#define TK_CUR_NUM(T) ((T)->cur->typ==TK_TT_DNUM || \
                       (T)->cur->typ==TK_TT_HNUM ||  \
                       (T)->cur->typ==TK_TT_RNUM)

/* return TRUE if the current token type is an integral number */
#define TK_CUR_INUM(T) ((T)->cur->typ==TK_TT_DNUM || \
                        (T)->cur->typ==TK_TT_HNUM)

/* return TRUE if the current token type is an instance qualifier */
#define TK_CUR_IQUAL(T) ((T)->cur->typ==TK_TT_QMARK || \
                         (T)->cur->typ==TK_TT_STAR ||  \
                         (T)->cur->typ==TK_TT_PLUS)


/* return TRUE if the current token type is an identifier */
#define TK_CUR_ID(T) ((T)->cur->typ==TK_TT_TSTRING || \
                      (T)->cur->typ==TK_TT_MSTRING)

/* return TRUE if the current token type is a scoped identifier */
#define TK_CUR_SID(T) ((T)->cur->typ==TK_TT_SSTRING || \
                       (T)->cur->typ==TK_TT_MSSTRING)

/* return the current line number */
#define TK_CUR_LNUM(T) ((T)->cur->linenum)

/* return the current line position */
#define TK_CUR_LPOS(T) ((T)->cur->linepos)

/* return TRUE if the token is a text or number type, 
 * as opposed to a 1 or 2 char non-text token type
 */
#define TK_CUR_TEXT(T) ((T)->cur->typ >= TK_TT_SSTRING &&\
			(T)->cur->typ <= TK_TT_RNUM)


/* bits for tk_chain_t flags field */
#define TK_FL_REDO    bit0


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* different types of tokens parsed during 1st pass */
typedef enum tk_type_t_ {
    TK_TT_NONE,
    /* PUT ALL 1-CHAR TOKENS FIRST */
    TK_TT_LBRACE,               /* left brace '{' */
    TK_TT_RBRACE,               /* right brace '}' */
    TK_TT_SEMICOL,              /* semi-colon ';' */
    TK_TT_LPAREN,               /* left paren '(' */
    TK_TT_RPAREN,               /* right paren ')' */
    TK_TT_LBRACK,               /* left bracket '[' */
    TK_TT_RBRACK,               /* right bracket ']' */
    TK_TT_COMMA,                /* comma ',' */ 
    TK_TT_EQUAL,                /* equal sign '=' */ 
    TK_TT_BAR,                  /* bar '|' */
    TK_TT_STAR,                 /* star '*' */
    TK_TT_ATSIGN,               /* at sign '@' */
    TK_TT_PLUS,                 /* plus mark '+' */
    TK_TT_COLON,                /* colon char ':' */
    TK_TT_PERIOD,               /* period char '.' */
    TK_TT_FSLASH,               /* forward slash char '/' */
    TK_TT_MINUS,                /* minus char '-' */
    TK_TT_LT,                   /* less than char '<' */
    TK_TT_GT,                   /* greater than char '>' */
    TK_TT_DOLLAR,               /* dollar sign '$' */

    /* PUT ALL 2-CHAR TOKENS SECOND */
    TK_TT_RANGESEP,             /* range sep, parent node '..' */
    TK_TT_DBLCOLON,             /* 2 colon chars '::' */
    TK_TT_DBLFSLASH,            /* 2 fwd slashes '//' */
    TK_TT_NOTEQUAL,             /* not equal '!=' */
    TK_TT_LEQUAL,               /* less than or equal '<=' */
    TK_TT_GEQUAL,               /* greater than or equal '>=' */


    /* PUT ALL STRING CLASSIFICATION TOKENS THIRD */
    TK_TT_STRING,               /* unquoted string */
    TK_TT_SSTRING,              /* scoped token string */
    TK_TT_TSTRING,              /* token string */
    TK_TT_MSTRING,              /* module-qualified token string */
    TK_TT_MSSTRING,             /* module-qualified scoped string */
    TK_TT_QSTRING,              /* double quoted string */
    TK_TT_SQSTRING,             /* single quoted string */

    /* PUT ALL NUMBER CLASSIFICATION TOKENS FOURTH */
    TK_TT_DNUM,                 /* decimal number */
    TK_TT_HNUM,                 /* hex number */
    TK_TT_RNUM,                 /* real number */

    /* PUT ALL SPECIAL CASE TOKENS LAST */
    TK_TT_NEWLINE               /* \n is significant in conf files */
} tk_type_t;

typedef enum tk_source_t_ {
    TK_SOURCE_CONF,
    TK_SOURCE_YANG,
    TK_SOURCE_XPATH,
    TK_SOURCE_REDO
} tk_source_t;


/* single NCX language token type */
typedef struct tk_token_t_ {
    dlq_hdr_t  qhdr;
    tk_type_t  typ;
    xmlChar    *mod;       /* only used if module qualifier found */
    uint32     modlen;
    xmlChar    *val;       /* only used for variable length tokens */
    uint32     len;
    uint32     linenum;
    uint32     linepos;
} tk_token_t;


/* token parsing chain */
typedef struct tk_chain_t_ {
    dlq_hdr_t      qhdr;
    dlq_hdr_t      tkQ;
    tk_token_t    *cur;
    const xmlChar *filename;
    FILE          *fp;
    xmlChar       *buff;
    xmlChar       *bptr;
    uint32         linenum;
    uint32         linepos;
    uint32         flags;
    tk_source_t    source;
} tk_chain_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/
extern tk_chain_t *
    tk_new_chain (void);

extern void
    tk_setup_chain_conf (tk_chain_t *tkc,
			 FILE *fp,
			 const xmlChar *filename);

extern void
    tk_setup_chain_yang (tk_chain_t *tkc,
			 FILE *fp,
			 const xmlChar *filename);

extern void
    tk_free_chain (tk_chain_t *tkc);

extern tk_type_t 
    tk_get_token_id (const xmlChar *buff, 
		     uint32 len);

/* checks for valid YANG data type name */
extern ncx_btype_t 
    tk_get_yang_btype_id (const xmlChar *buff, 
			  uint32 len);

extern const char *
    tk_get_btype_sym (ncx_btype_t btyp);

extern const char *
    tk_get_token_name (tk_type_t ttyp);

extern const char *
    tk_get_token_sym (tk_type_t ttyp);

extern tk_type_t
    tk_next_typ (tk_chain_t *tkc);

extern const xmlChar *
    tk_next_val (tk_chain_t *tkc);

extern void
    tk_dump_chain (tk_chain_t *tkc);

extern boolean
    tk_is_wsp_string (const tk_token_t *tk);

/* mod used just for additional error info, may be NULL */
extern status_t 
    tk_tokenize_input (tk_chain_t *tkc,
		       ncx_module_t *mod);


/* mod used just for additional error info, may be NULL */
extern status_t 
    tk_retokenize_cur_string (tk_chain_t *tkc,
			      ncx_module_t *mod);

/* convert the ncx:metadata content to 1 or 2 tokens */
extern tk_chain_t *
    tk_tokenize_metadata_string (ncx_module_t *mod,
				 xmlChar *str,
				 status_t *res);

/* convert an XPath string to tokens
 * mod can be NULL -- used for error reporting
 */
extern tk_chain_t *
    tk_tokenize_xpath_string (ncx_module_t *mod,
			      xmlChar *str,
			      uint32 curlinenum,
			      uint32 curlinepos,
			      status_t *res);



extern uint32
    tk_token_count (const tk_chain_t *tkc);

extern void
    tk_reset_chain (tk_chain_t *tkc);

#endif	    /* _H_tk */
