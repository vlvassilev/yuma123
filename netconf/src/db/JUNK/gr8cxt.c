/*  FILE: gr8cxt.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
18-apr-96    abb      begin gr8cgi -- based on Rob McCool's
                      post-query program and some of my basic
		      data-mgmt library stuff (tbd: gsql)
27-apr-96    abb      split into gr8cgi and gr8cxt to reduce coupling
                      and image size
14-sep-01    abb      move some parameters to SQL storage and
                      remove usage of w3-mysql-abb. All pages
                      now registered in pagenav and handled through
                      this program

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <unistd.h>

/* this is the main module--allocate storage for global variables */
#define	_C_main    	1

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_db
#include  "db.h"
#endif

#ifndef _H_dlq
#include  "dlq.h"
#endif

#ifndef _H_catvars
#include "catvars.h"
#endif

#ifndef _H_parsparms
#include "parsparms.h"
#endif

#ifndef _H_cxt
#include "cxt.h"
#endif

#ifndef _H_gr8pag
#include "gr8pag.h"
#endif

#ifndef _H_gr8form
#include "gr8form.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#define VALBUFF_LEN               0xC000
#define CXTSTR_BUFLEN             0x10

/* CPP_DEBUG_DUMP -- define to dump envVars for debugging */
/* #define CPP_DEBUG_DUMP 1 */
/* CPP_DEBUG_POST -- define to debug form input via POST or GET */
/* #define CPP_DEBUG_POST 1 */

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
static  dlq_hdrT    valParseQ;
static  char        valbuff[VALBUFF_LEN];
static  db_order_t  order_rec;


/********************************************************************
*                                                                   *
*		FUNCTION free_globals
*                                                                   *
*********************************************************************/
static void free_globals(void)
{
    parsparms_freeValList(&valParseQ);
    db_close();
}


/********************************************************************
*                                                                   *
*		FUNCTION init_globals
*                                                                   *
*********************************************************************/
static status_t init_globals (void)
{
    status_t res;

    pgRequestMethod = getenv("REQUEST_METHOD");
    pgServerName = getenv("SERVER_NAME");
    pgQueryString = getenv("QUERY_STRING");
    pgRemoteAddr = getenv("REMOTE_ADDR");
    pgDocumentUri = getenv("DOCUMENT_URI");
    pgDocumentRoot = getenv("DOCUMENT_ROOT");
    pgContentType = getenv("CONTENT_TYPE");
    pgContentLength = getenv("CONTENT_LENGTH");
    pgAuthType = getenv("AUTH_TYPE");
    pgServerSoftware = getenv("SERVER_SOFTWARE");
    pgGatewayInterface = getenv("GATEWAY_INTERFACE");
    pgServerProtocol = getenv("SERVER_PROTOCOL");
    pgServerPort = getenv("SERVER_PORT");
    pgHttpAccept = getenv("HTTP_ACCEPT");
    pgPathInfo = getenv("PATH_INFO");
    pgPathTranslated = getenv("PATH_TRANSLATED");
    pgScriptName = getenv("SCRIPT_NAME");
    pgRemoteHost = getenv("REMOTE_HOST");
    pgDocumentName = getenv("DOCUMENT_NAME");
    pgRemoteUser = getenv("REMOTE_USER");

    gContentTypePrinted = FALSE;
    gErrPagePrinted = FALSE;

    /* open up the product database */
    res = db_open();
    if (res==NO_ERR)  {
	res = db_getServerSettings(&gSettings);
	if (res==NO_ERR) {
	    res = db_getVendorSettings(&gVendor);
	}
    }
    if (res != NO_ERR) {
	db_close();  /* okay to call even if db_open fails */
    }
    return res;
}


#ifdef CPP_DEBUG_DUMP
/********************************************************************
*                                                                   *
*		FUNCTION dump_env_vars		                    *
*                                                                   *
*********************************************************************/
static void dump_env_vars(void)
{
    if (!gContentTypePrinted) {
	printf("Content-type: text/html\n\n");
	gContentTypePrinted = TRUE;
    }
    printf("<pre>\n");
    printf("%s = %s\n", "pgRequestMethod", pgRequestMethod ?
	   pgRequestMethod : "null");
    printf("%s = %s\n", "pgContentType", pgContentType ?
	   pgContentType : "null");
    printf("%s = %s\n", "pgContentLength", pgContentLength ?
	   pgContentLength : "null");
    printf("%s = %s\n", "pgAuthType", pgAuthType ?
	   pgAuthType : "null");
    printf("%s = %s\n", "pgServerSoftware", pgServerSoftware ?
	   pgServerSoftware : "null");
    printf("%s = %s\n", "pgServerName", pgServerName ?
	   pgServerName : "null");
    printf("%s = %s\n", "pgGatewayInterface", pgGatewayInterface ?
	   pgGatewayInterface : "null");
    printf("%s = %s\n", "pgServerProtocol", pgServerProtocol ?
	   pgServerProtocol : "null");
    printf("%s = %s\n", "pgServerPort", pgServerPort ?
	   pgServerPort : "null");
    printf("%s = %s\n", "pgHttpAccept", pgHttpAccept ?
	   pgHttpAccept : "null");
    printf("%s = %s\n", "pgPathInfo", pgPathInfo ?
	   pgPathInfo : "null");
    printf("%s = %s\n", "pgPathTranslated", pgPathTranslated ?
	   pgPathTranslated : "null");
    printf("%s = %s\n", "pgScriptName", pgScriptName ?
	   pgScriptName : "null");
    printf("%s = %s\n", "pgQueryString", pgQueryString ?
	   pgQueryString : "null");
    printf("%s = %s\n", "pgRemoteHost", pgRemoteHost ?
	   pgRemoteHost : "null");
    printf("%s = %s\n", "pgRemoteAddr", pgRemoteAddr ?
	   pgRemoteAddr : "null");
    printf("%s = %s\n", "pgRemoteUser", pgRemoteUser ?
	   pgRemoteUser : "null");
    printf("%s = %s\n", "pgDocumentName", pgDocumentName ?
	   pgDocumentName : "null");
    printf("%s = %s\n", "pgDocumentUri", pgDocumentUri ?
	   pgDocumentUri : "null");
    printf("%s = %s\n", "pgDocumentRoot", pgDocumentRoot ?
	   pgDocumentRoot : "null");

    printf("</pre>\n");
}
#endif  /* CPP_DEBUG_DUMP */


/********************************************************************
*                                                                   *
*			FUNCTION handle_get			    *
*                                                                   *
*********************************************************************/
status_t handle_get (void)
{
    int                  clen = 0;
    status_t             res;
    const char           *pstr = NULL;

    res = db_getPagenav(pgDocumentUri, &gPage);
    if (res != NO_ERR) {
	return res;
    }
	
    /* read in all QUERY_STRING data into valParseQ */
    if (pgQueryString && (clen = strlen(pgQueryString)))  {
        res = parsparms_readGetInput(pgQueryString, clen, &valParseQ);
	if (res==NO_ERR) {
	    /* there were some QUERY_STRING params, so first check
	     * for the CXT tag in the QUERY_STRING, and if it's there
	     * check to see if it represents a valid order
	     */
#ifdef CPP_DEBUG_DUMP
	    parsparms_dumpValList(&valParseQ);
#endif
	    pstr = parsparms_findVal(cxt_getTagName(), &valParseQ);
	}
    }

    /* look for an open order if a CXT string is found */
    if (pstr) {  
        if (!cxt_labelIsCurrent(pstr, &order_rec)) {
	    db_initNullOrder(&order_rec);  
        }  /* else found the current order, now in order_rec */
    } else {                              /* no CXT ID found */
	db_initNullOrder(&order_rec);
    }

    /* If we get here -- order found or created
     * and order_rec is filled in at this point;
     * check if this is a Greytsounds C-generated page 
     */
    return gr8pag_emitGr8Page(pgDocumentUri, &order_rec, 
			     &valParseQ);
}


/********************************************************************
*                                                                   *
*			FUNCTION handle_post			    *
*                                                                   *
*********************************************************************/
static status_t handle_post (void)
{
    status_t		 res;
    int                  clen;

#ifdef CPP_DEBUG_POST
    /* TRUE  == take input from QUERY_STRING 
     * FALSE == take input from STDIN (normal POST mode)
     */
    boolean              debug_post_switch_input = TRUE;
#endif

    /* FORM ACTION function:
     * Check the content-type and get the content-length 
     * Read the parameter input from stdin into the pars
     * Extract the CXT id and db-lookup the CXT.PG record
     * Dispatch to form handler to update the CXT.PG record
     *   possibly other output (other files, email, etc.),
     *   and output the 'next page'
     */
#ifdef CPP_DEBUG_POST
    if (!debug_post_switch_input) {
#endif
        clen = (pgContentLength) ? atoi(pgContentLength) : 0;
	if (!pgContentType ||
	    strcmp(pgContentType,"application/x-www-form-urlencoded") || 
	    !clen)  {
	    set_error(__FILE__, __LINE__, ERR_INTERNAL_VAL, 0);
	    return ERR_INTERNAL_VAL;
	}
#ifdef CPP_DEBUG_POST
    }
#endif

    /* check for debug POST input mode and if TRUE,
     * convert input to a QUERY_STRING
     */
#ifdef CPP_DEBUG_POST
    if (debug_post_switch_input) {
	/* hack a test with QUERY_STRING instead of STDIN for debugging */
	if (pgQueryString != NULL) {
	    res=parsparms_readGetInput(pgQueryString, 
	       strlen(pgQueryString), &valParseQ);
	} else {
	    set_error(__FILE__, __LINE__, ERR_INTERNAL_VAL, 0);
	    res = ERR_INTERNAL_VAL;
	}
    } else {    
	/* normal POST mode 
	 * -- read the param list passed from the httpd server 
	 */
	res=parsparms_readPostInput(stdin, clen, 
	    valbuff, VALBUFF_LEN, &valParseQ);
    }
#else
    res = parsparms_readPostInput(stdin, clen, 
				valbuff, VALBUFF_LEN, &valParseQ);
#endif  /* CPP_DEBUG_POST */

    if (res != NO_ERR) {
	return res;
    }

#ifdef CPP_DEBUG_DUMP
    parsparms_dumpValList(&valParseQ);
#endif

    /* need to determine the FORM_ID and CXT tag 
     * and dispatch to the form handler 
     * pass the static order record buffer to prevent
     * more than one huge static buffer
     * order_rec empty at this point
     */
    return gr8form_handleForm(&valParseQ, &order_rec);
}

/********************************************************************
*                                                                   *
*			FUNCTION main				    *
*                                                                   *
*********************************************************************/
int main (int argc, char *argv[])
{
    status_t		 res;

    /* init module vars */
    dlq_createSQue(&valParseQ);

    res = init_globals();

#ifdef CPP_DEBUG_DUMP
    dump_env_vars();   /* okay even if init_globals failed */
#endif

    /* check if the database opened okay from init_globals call */
    if (res == NO_ERR) {
        if (pgRequestMethod != NULL) {
	    /* check required vars */
	    if (!strcmp(pgRequestMethod, "GET")) {
	        res = handle_get();
	    } else if (!strcmp(pgRequestMethod, "POST")) {
	        res = handle_post();
	    } else {
                set_error(__FILE__, __LINE__, ERR_INTERNAL_VAL, 0);
		res = ERR_INTERNAL_VAL;
	    }
	} else {
  	    res = ERR_INTERNAL_PTR;
	}
    }

    if (res != NO_ERR) {
        /* print a catchall error page in case one hasn't
	 * been printed already
	 */
        gr8pag_emitErrorPage("Error Processing Request", res, NULL, NULL);
    }


    free_globals();
    return 0;    /* always return OK */
}

/* END gr8cxt.c */



