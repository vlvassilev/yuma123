/*  FILE: rpc_mgr.c

   NETCONF Protocol Operations: RPC Manager Side Support

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
05may05      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <memory.h>

#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_ncx
#include  "ncx.h"
#endif

#ifndef _H_rpc
#include  "rpc.h"
#endif

#ifndef _H_rpc_mgr
#include  "rpc_mgr.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif

#ifndef _H_xml_util
#include  "xml_util.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/* these constants could be raised for really better performance
 * on a large number of concurrent RPC requests
 */
#define RPC_MGR_START_FREE  8
#define RPC_MGR_MAX_FREE    32
#define RPC_MGR_RING_SIZE   128

#define RPC_MGR_DEF_SIZE    1536

/********************************************************************
*                                                                   *
*                           T Y P E S                               *
*                                                                   *
*********************************************************************/
    

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
static uint32        next_msg_id = 1;
static dlq_hdr_t     reqs[RPC_MGR_RING_SIZE];
static dlq_hdr_t     free_q;
static dlq_hdr_t     alloc_q;
static uint32        free_cnt;
static uint32        alloc_cnt;
static boolean       shutting_down;
static boolean       rpc_mgr_init_done = FALSE;


/********************************************************************
* FUNCTION create_req
*
* Create an RPC request structure
*
* INPUTS:
*   reqsize == requested size for rpc+payload buffer
* RETURNS:
*   pointer to request buf or NULL if a malloc error 
*********************************************************************/
static rpc_msg_t * create_req (uint32 reqsize)
{
    rpc_msg_t  *req;
    status_t   res;

    req = rpc_new_msg();

    req->rpc_incoming = FALSE;
    req->rpc_state = RPC_ST_INIT;

    res = buf_init_buffer(&req->rpc_buff, reqsize);
    if (res != NO_ERR) {
	m__free(req);
	return NULL;
    }
    req->rpc_binit = TRUE;

    alloc_cnt++;
    return req;

} /* create_req */


/********************************************************************
* FUNCTION insert_free
*
* Insert a free entry in the sorted free_q
*
* INPUTS:
*   req == entry to insert
* RETURNS:
*   none
*********************************************************************/
static void insert_free (rpc_msg_t  *req)
{
    rpc_msg_t  *fr;

    /* look for the first entry bigger than the entry to be freed */
    for (fr = (rpc_msg_t *)dlq_firstEntry(&free_q);
	 fr != NULL;
	 fr = (rpc_msg_t *)dlq_nextEntry(fr)) {
	if (buf_maxlen(&fr->rpc_buff) >= buf_maxlen(&req->rpc_buff)) {
	    dlq_insertAhead(req, fr);
	    return;
	}
    }

    /* no insert point found -- new last entry */
    dlq_enque(req, &free_q);
    free_cnt++;

} /* insert_free */


/********************************************************************
* FUNCTION free_req
*
*   Free an RPC request PDU
*   Does not remove from any queue!! 
*   That must be done before this function is called.
*
* INPUTS:
*   req == pointer to rpc_msg_t to free
* RETURNS:
*   none
*********************************************************************/
static void free_req (rpc_msg_t *req)
{
    if (!shutting_down && free_cnt < RPC_MGR_MAX_FREE) {
	/* put this in the free_q instead of deleting it */
	rpc_clean_msg(req);
	req->rpc_state = RPC_ST_FREE;
	insert_free(req);
    } else {
	/* free_q full or shutting down -- delete it */
	rpc_free_msg(req);
	alloc_cnt--;
    }
}  /* free_req */


/********************************************************************
* FUNCTION find_free_req
*
* Find a request PDU of at least the specified size
*
* INPUTS:
*   reqsize == requested size for rpc+payload buffer
* RETURNS:
*   pointer to request buf or NULL if none found
*********************************************************************/
static rpc_msg_t * find_free_req (uint32 reqsize)
{
    rpc_msg_t  *req;

    for (req = (rpc_msg_t *)dlq_firstEntry(&free_q);
	 req != NULL;
	 req = (rpc_msg_t *)dlq_nextEntry(req)) {
	if (buf_maxlen(&req->rpc_buff) >= reqsize) {
	    dlq_remove(req);
	    free_cnt--;
	    return req;
	}
    }
    return NULL;

} /* find_free_req */


#ifdef NOT_YET
/********************************************************************
* FUNCTION find_active_req
*
* Find a request PDU in the active requests array
* This function does not remove the request from the
* active request array.
*
* INPUTS:
*   msg-id == the message-id attribute to find
* RETURNS:
*   pointer to request buf or NULL if not found
*********************************************************************/
static rpc_msg_t * find_active_req (const xmlChar *msg_id)
{
    rpc_msg_t  *req;


    for (req = (rpc_msg_t *)
	     dlq_firstEntry(&reqs[msg_id % RPC_MGR_RING_SIZE]);
	 req != NULL;
	 req = (rpc_msg_t *)dlq_nextEntry(req)) {
	if (!xml_strcmp(req->rpc_msg_id, msg_id) {
	    return req;
	}
    }
    return NULL;

} /* find_active_req */
#endif


/********************************************************************
* FUNCTION rpc_mgr_init
*
* Initialize the rpc_mgr module data
*
* INPUTS:
*   none
* RETURNS:
*   NO_ERR if all okay, the minimum spare requests will be malloced
*********************************************************************/
status_t rpc_mgr_init (void)
{
    uint32 i;
    rpc_msg_t    *req;

    if (rpc_mgr_init_done) {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    /* initialize the requests-in-progress array of queues */
    for (i=0; i<RPC_MGR_RING_SIZE; i++) {
	dlq_createSQue(&reqs[i]);
    }

    /* init the queue of free requests */
    dlq_createSQue(&free_q);
    free_cnt = 0;

    dlq_createSQue(&alloc_q);
    alloc_cnt = 0;

    /* create the minimum free request buffers */
    for (i=0; i<RPC_MGR_START_FREE; i++) {
	req = create_req(RPC_MGR_DEF_SIZE);
	if (!req) {
	    return SET_ERROR(ERR_INTERNAL_MEM);
	}
	dlq_enque(req, &free_q);
    }
    free_cnt = RPC_MGR_START_FREE;
    shutting_down = FALSE;

    rpc_mgr_init_done = TRUE;
    return NO_ERR;

} /* rpc_mgr_init */


/********************************************************************
* FUNCTION rpc_mgr_cleanup
*
* Cleanup the rpc_mgr module data
*
* INPUTS:
*   none
* RETURNS:
*   none
*********************************************************************/
void rpc_mgr_cleanup (void)
{
    uint32 i;
    rpc_msg_t    *req;

    if (rpc_mgr_init_done) {
        shutting_down = TRUE;

        for (i=0; i<RPC_MGR_RING_SIZE; i++) {
            while ((req = dlq_deque(&reqs[i])) != NULL) {
                free_req(req);
            }
        }

        while ((req = dlq_deque(&alloc_q)) != NULL) {
            free_req(req);
        }
        alloc_cnt = 0;

        while ((req = dlq_deque(&free_q)) != NULL) {
            free_req(req);
        }
        free_cnt = 0;

        rpc_mgr_init_done = FALSE;
    }
} /* rpc_mgr_cleanup */


/********************************************************************
* FUNCTION rpc_mgr_alloc_req
*
*   Allocate an RPC request PDU
*
* INPUTS:
*   reqsize == requested message size
*   group_id == request group ID (only generated if non-zero)
* RETURNS:
*   pointer to the allocated request or NULL if allocation failed
*********************************************************************/
rpc_msg_t * rpc_mgr_alloc_req (uint32 reqsize,
			       uint32 group_id)
{
    rpc_msg_t  *req;
    unsigned char  num[12];
    status_t    res;

    if (!rpc_mgr_init_done) {
        res = rpc_mgr_init();
        if (res != NO_ERR) {
            return NULL;
        }
    }

    if (!reqsize) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }

    /* try to get a free request PDU */
    req = find_free_req(reqsize);
    if (!req) {
	/* create a new request instead */
	req = create_req(reqsize);
	if (!req) {
	    SET_ERROR(ERR_INTERNAL_MEM);
	    return NULL;
	}
    }

    /* set the message type */
    req->rpc_incoming = FALSE;

    /* setup the attribute queue */
    xml_init_attrs(&req->rpc_attrs);

    /* add the netconf xml attribute */
    res = xml_add_attr(&req->rpc_attrs, xmlns_nc_id(), XMLNS, NC_URN);
    if (res != NO_ERR) {
	free_req(req);
	return NULL;
    }

    /* add the message-id attribute */
    xmlStrPrintf(num, 12, (const xmlChar *) "%lu", next_msg_id);
    res = xml_add_attr(&req->rpc_attrs, xmlns_nc_id(), 
                         (const xmlChar *) RPC_STR_MSG_ID, num);
    if (res != NO_ERR) {
	free_req(req);
	return NULL;
    }
	
    /* maybe add the ncx NS and group-id attributes */
    if (group_id) {
	/* add the netconf-extensions xmlns attribute */
	res = xml_add_attr(&req->rpc_attrs, xmlns_ncx_id(), 
               XMLNS, NCX_URN);
	if (res != NO_ERR) {
	    free_req(req);
	    return NULL;
	}

	/* add the group-id attribute */
	xmlStrPrintf(num, 12, (const xmlChar *) "%lu", group_id);
	res = xml_add_attr(&req->rpc_attrs, xmlns_ncx_id(), 
             (const xmlChar *) RPC_STR_GRP_ID, num);
	if (res != NO_ERR) {
	    free_req(req);
	    return NULL;
	}
    }

    req->rpc_msg_id = num;
    next_msg_id++;
    req->rpc_group.u = group_id;
    req->rpc_state = RPC_ST_ALLOC;

    if (!next_msg_id) {   /* msg ID wrapped to 0, will never happen */
	next_msg_id++;     
    }

    dlq_enque(req, &alloc_q);
    
    return req;

}  /* rpc_mgr_alloc_req */


/********************************************************************
* FUNCTION rpc_mgr_free_req
*
*   Free an RPC request PDU
*
* INPUTS:
*   req == pointer to rpc_msg_t to free
* RETURNS:
*   none
*********************************************************************/
void rpc_mgr_free_req (rpc_msg_t *req)
{
    status_t  res;

    if (!rpc_mgr_init_done) {
        res = rpc_mgr_init();
        if (res != NO_ERR) {
            SET_ERROR(res);
            return;
        }
    }

    if (!req) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }

    /* check if the entry needs to be removed from some queue */
    switch (req->rpc_state) {
    case RPC_ST_NONE:
    case RPC_ST_RPY:
	break;
    case RPC_ST_FREE:
    case RPC_ST_ALLOC:
    case RPC_ST_SENT:
	dlq_remove(req);
    default:
	break;
    }
    free_req(req);

}  /* rpc_mgr_free_req */


/********************************************************************
* FUNCTION rpc_mgr_start_hdr
*
* Write the rpc start tag to a buffer
*
* The caller will set up an rpc_msg_t struct and a queue of
* attributes, including a message ID 
*
* INPUTS:
*   ns_id == namespace ID
*   attrs == rpc element attributes
*            THIS LIST MUST HAVE THE message-id ATTRIBUTE 
*            AND the netconf xmlns declaration
*   buffer == buffer to fill
* OUTPUTS:
*   buffer is written with <rpc> element if NO_ERR
* RETURNS:
*     NO_ERR if all okay
*********************************************************************/
status_t rpc_mgr_start_hdr (
			    xmlns_id_t      ns_id,
			    xml_attrs_t  *attrs,
			    buf_buffer_t   *buffer)
{
    status_t  res;


    if (!buffer) {
	return SET_ERROR(ERR_INTERNAL_PTR);    
    }

    res = buf_write_tag(buffer, ns_id, (const xmlChar *) RPC_STR_REQ, 
                        BUF_START_TAG, attrs);
    if (res != NO_ERR) {
	return res;
    }

    buf_inc_indent_level(buffer);
    return NO_ERR;

}  /* rpc_mgr_start_hdr */


/********************************************************************
* FUNCTION rpc_mgr_finish_hdr
*
*   Write the rpc end tag to a buffer
*
* INPUTS:
*   ns_id == namespace ID
*   buffer == buffer to fill
* OUTPUTS:
*   buffer is written with </rpc> tag if NO_ERR
* RETURNS:
*     NO_ERR if all okay
*********************************************************************/
status_t rpc_mgr_finish_hdr (
			     xmlns_id_t          ns_id,
			     buf_buffer_t       * buffer)
{
    if (!buffer) {
	return SET_ERROR(ERR_INTERNAL_PTR);    
    }

    buf_dec_indent_level(buffer);

    return buf_write_tag(buffer, ns_id, (const xmlChar *) RPC_STR_REQ, 
        BUF_END_TAG, NULL);

}  /* rpc_mgr_finish_hdr */


/* END file rpc_mgr.c */
