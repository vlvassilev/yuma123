/*    File dlq.c

      dlq provides general queue and linked list support:

       QUEUE initialization/cleanup
       * create queue	(create and initialize dynamic queue hdr)
       * destroy queue	(destroy previously created dynamic queue)	   
       * create Squeue	(initialize static queue hdr--no destroyS function)

       FIFO queue operators
       * enque	     (add node to end of list)
       * deque	     (return first node - remove from list)
       * empty	     (return TRUE if queue is empty, FALSE otherwise)

       LINEAR search (linked list)
       * nextEntry   (return node AFTER param node - leave in list)
       * prevEntry   (return node BEFORE param node - leave in list)
       * insertAhead (add node in list ahead of param node)
       * insertAfter (add node in list after param node)
       * remove	     (remove a node from a linked list)


*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
07-jan-89    abb      Begun.
24-mar-90    abb      Add que_firstEntry routine
18-feb-91    abb      Add que_createSQue function
12-mar-91    abb      Adapt for DAVID sbee project
13-jun-91    abb      change que.c to dlq.c
22-oct-93    abb      added critical section hooks
14-nov-93    abb      moved to this library from \usr\src\gendep\1011
15-jan-07    abb      update for NCX software project 
                      add dlq_swap, dlq_hdr_t
                      change err_msg to use log_error function
12-oct-07    abb      add dlq_count
*/

/********************************************************************
*								    *
*		      I N C L U D E    F I L E S		    *
*								    *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_dlq
#include  "dlq.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#define	    err_msg(S)	log_error("\nerr: %s ",#S)


/* add enter and exit critical section hooks here */
#ifndef ENTER_CRIT
#define	ENTER_CRIT
#endif

#ifndef EXIT_CRIT
#define EXIT_CRIT
#endif


#ifdef CPP_DEBUG    
/********************************************************************
*								    *
*			function dlq_dumpHdr			    *
*								    *
*********************************************************************/
void dlq_dumpHdr (const void *nodeP)
{
    const dlq_hdrT   *p = (const dlq_hdrT *) nodeP;

    if (p==NULL)
    {
	log_debug("\ndlq: NULL hdr");
	return;
    }
    log_debug("\ndlq: ");
    switch(p->hdr_typ)
    {
    case DLQ_NULL_NODE:
	log_debug("unused	 ");
	break;
    case DLQ_SHDR_NODE:
	log_debug("shdr node");
	break;
    case DLQ_DHDR_NODE:
	log_debug("dhdr node");
	break;
    case DLQ_DATA_NODE:
	log_debug("data node");
	break;
    case DLQ_DEL_NODE:
	log_debug("deleted	 ");
	break;
    default:
	log_debug("invalid	 ");
    }
    
    log_debug("(%p) p (%p) n (%p)", p, p->prev, p->next);

}   /* END dlq_dumpHdr */
#endif	    /* CPP_DEBUG */



/********************************************************************
*								    *
*			function dlq_createQue			    *
*								    *
*********************************************************************/
dlq_hdrT * dlq_createQue (void)
{
    REG dlq_hdrT  *retP;

    retP = m__getObj(dlq_hdrT);
    if (retP==NULL) {
	return NULL;
    }

    /* init empty list */
    retP->hdr_typ = DLQ_DHDR_NODE;
    retP->prev = retP->next = retP;
    return retP;

}    /* END function dlq_createQue */



/********************************************************************
*								    *
*			function dlq_createSQue			    *
*								    *
*********************************************************************/
void  dlq_createSQue (dlq_hdrT *queAddr)
/* create static queue--init 'queAddr'	*/
{
#ifdef CPP_ICHK
    if (queAddr==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return;
    }
#endif
    
    /* init empty list */
    ENTER_CRIT;
    queAddr->hdr_typ = DLQ_SHDR_NODE;
    queAddr->prev = queAddr->next = queAddr;
    EXIT_CRIT;
    
}    /* END function dlq_createSQue */



/********************************************************************
*								    *
*		     function dlq_destroyQue			    *
*								    *
*********************************************************************/
void dlq_destroyQue (dlq_hdrT *listP)
{
#ifdef CPP_ICHK
    if (listP==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return;
    }
    if (!_hdr_node(listP) || !dlq_empty(listP))
    {
	err_msg(ERR_INTERNAL_QDEL);
	return;
    }
#endif

    if (listP->hdr_typ==DLQ_DHDR_NODE)
    {
	listP->hdr_typ = DLQ_DEL_DHDR;
	m__free(listP);
    }
    
}    /* END function dlq_destroyQue */



/********************************************************************
*								    *
*			 function dlq_enque			    *
*								    *
*********************************************************************/
void dlq_enque (REG void *newP, REG dlq_hdrT *listP)
{
#ifdef CPP_ICHK
    if (newP==NULL || listP==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return;
    }
    else if (!_hdr_node(listP))
    {
	err_msg(ERR_QNODE_NOT_HDR);
	return;
    }
#endif

    ENTER_CRIT;
    /* code copied directly form dlq_insertAhead to save a little time */
    ((dlq_hdrT *) newP)->hdr_typ = DLQ_DATA_NODE;
    ((dlq_hdrT *) newP)->next = listP;
    ((dlq_hdrT *) newP)->prev = ((dlq_hdrT *) listP)->prev;
    ((dlq_hdrT *) newP)->next->prev = newP;
    ((dlq_hdrT *) newP)->prev->next = newP;
    EXIT_CRIT;
    
}    /* END function dlq_enque */



/********************************************************************
*								    *
*		      function dlq_deque			    *
*								    *
*********************************************************************/
void  *dlq_deque (dlq_hdrT *  listP)
{
    REG void   *nodeP;

#ifdef CPP_ICHK
    if (listP==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return NULL;
    }
    else if (!_hdr_node(listP))
    {
	err_msg(ERR_QNODE_NOT_HDR);
	return NULL;
    }
#endif

    ENTER_CRIT;
    /* check que empty */
    if (listP==listP->next)
    {
	EXIT_CRIT;
	return NULL;
    }
    
    /* 
     * return next que element, after removing it from the que
     * the que link ptrs in 'nodeP' are set to NULL upon return
     * the dlq_hdr in 'nodeP' is also marked as deleted
     */

    nodeP = listP->next;
#ifdef CPP_ICHK
    if (!_data_node(nodeP))
    {
	EXIT_CRIT;
	err_msg(ERR_QNODE_NOT_DATA);
	return NULL;
    }
#endif

    /*** dlq_remove (listP->next); *** copied inline below ***/

    /* relink the queue chain */
    ((dlq_hdrT *) nodeP)->prev->next = ((dlq_hdrT *) nodeP)->next;
    ((dlq_hdrT *) nodeP)->next->prev = ((dlq_hdrT *) nodeP)->prev;

    /* remove the node from the linked list */
    ((dlq_hdrT *) nodeP)->hdr_typ = DLQ_DEL_NODE;
    ((dlq_hdrT *) nodeP)->prev	 = NULL;
    ((dlq_hdrT *) nodeP)->next	 = NULL;
    EXIT_CRIT;
    
    return nodeP;

}    /* END function dlq_deque */



#if defined(CPP_NO_MACROS)
/********************************************************************
*								    *
*		      function dlq_nextEntry			    *
*								    *
*********************************************************************/
void  *dlq_nextEntry (const void *nodeP)
{
    void    *retP;
    
#ifdef CPP_ICHK 
    if (nodeP==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return NULL;	/* error */
    }
#endif

    ENTER_CRIT;
    /* get next entry in list -- maybe	(hdr_node==end of list) */
    nodeP = ((dlq_hdrT *) nodeP)->next;

#ifdef CPP_ICHK
    if (!(_data_node(nodeP) || _hdr_node(nodeP)))
    {
	EXIT_CRIT;
	err_msg(ERR_BAD_QLINK);
	return NULL;
    }
#endif
    
    retP = _data_node( ((dlq_hdrT *) nodeP) ) ? nodeP : NULL;
    EXIT_CRIT;
    
    return retP;

}   /* END function dlq_nextEntry */
#endif	    /* CPP_NO_MACROS */


#if defined(CPP_NO_MACROS)
/********************************************************************
*								    *
*		      function dlq_prevEntry			    *
*								    *
*********************************************************************/
void  *dlq_prevEntry (const void *nodeP)
{
    void    *retP;
    
#ifdef CPP_ICHK
    if (nodeP==NULL)   
    {
	err_msg(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif
    ENTER_CRIT;
    /* get prev entry in list -- maybe	(hdr_node==start of list) */
    nodeP = ((dlq_hdrT *) nodeP)->prev;

#ifdef CPP_ICHK
    if (!(_data_node(nodeP) || _hdr_node(nodeP)))
    {
	EXIT_CRIT;
	err_msg(ERR_BAD_QLINK);
	return NULL;
    }
#endif
    
    retP = _data_node( ((dlq_hdrT *) nodeP) ) ? nodeP : NULL;
    EXIT_CRIT;

    return retP;
    
}   /* END function dlq_prevEntry */
#endif	    /* CPP_NO_MACROS */



/********************************************************************
*								    *
*		     function dlq_insertAhead			    *
*								    *
*********************************************************************/
void dlq_insertAhead (void *newP, void *nodeP)
{
#ifdef CPP_ICHK
    if (nodeP==NULL || newP==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return;
    }
    if (nodeP==newP)
    {
	err_msg(ERR_INTERNAL_VAL);
	return;
    }
#endif
    ENTER_CRIT;
    ((dlq_hdrT *) newP)->hdr_typ	    = DLQ_DATA_NODE;
    ((dlq_hdrT *) newP)->next	    = nodeP;
    ((dlq_hdrT *) newP)->prev	    = ((dlq_hdrT *) nodeP)->prev;
    ((dlq_hdrT *) newP)->next->prev  = newP;
    ((dlq_hdrT *) newP)->prev->next  = newP;
    EXIT_CRIT;
    
}    /* END function dlq_insertAhead */



/********************************************************************
*								    *
*		     function dlq_insertAfter			    *
*								    *
*********************************************************************/
void dlq_insertAfter (void *newP, void *nodeP)
{
#ifdef CPP_ICHK
    if (nodeP==NULL || newP==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return;
    }
    if (nodeP==newP)
    {
	err_msg(ERR_INTERNAL_VAL);
	return;
    }
#endif

    ENTER_CRIT;    
    ((dlq_hdrT *) newP)->hdr_typ	    = DLQ_DATA_NODE;
    ((dlq_hdrT *) newP)->prev	    = nodeP;
    ((dlq_hdrT *) newP)->next	    = ((dlq_hdrT *) nodeP)->next;
    ((dlq_hdrT *) newP)->next->prev  = newP;
    ((dlq_hdrT *) newP)->prev->next  = newP;
    EXIT_CRIT;
    
}    /* END function dlq_insertAfter */



/********************************************************************
*								    *
*			    function dlq_remove			    *
*								    *
*********************************************************************/
void dlq_remove (void *nodeP)
{
#ifdef CPP_ICHK
    if (nodeP==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return;
    }
    else if (!_data_node( ((dlq_hdrT *) nodeP) ))
    {
	err_msg(ERR_QNODE_NOT_DATA);
	return;
    }
#endif

    ENTER_CRIT;
    /* relink the queue chain */
    ((dlq_hdrT *) nodeP)->prev->next = ((dlq_hdrT *) nodeP)->next;
    ((dlq_hdrT *) nodeP)->next->prev = ((dlq_hdrT *) nodeP)->prev;

    /* remove the node from the linked list */
    ((dlq_hdrT *) nodeP)->hdr_typ = DLQ_DEL_NODE;
    ((dlq_hdrT *) nodeP)->prev	 = NULL;
    ((dlq_hdrT *) nodeP)->next	 = NULL;
    EXIT_CRIT;
    
}   /* END fuction dlq_remove */



/********************************************************************
*								    *
*			    function dlq_remove			    *
*								    *
*********************************************************************/
void dlq_swap (void *new_node, void *cur_node)
{
#ifdef CPP_ICHK
    if (new_node==NULL || cur_node==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return;
    }
    else if (!_data_node( ((dlq_hdrT *) cur_node) ))
    {
	err_msg(ERR_QNODE_NOT_DATA);
	return;
    }
#endif

    ENTER_CRIT;

    /* replace cur_node with new_node in the queue chain */
    ((dlq_hdrT *) cur_node)->prev->next = new_node;
    ((dlq_hdrT *) new_node)->prev = ((dlq_hdrT *) cur_node)->prev;
    ((dlq_hdrT *) cur_node)->prev = NULL;

    ((dlq_hdrT *) cur_node)->next->prev = new_node;
    ((dlq_hdrT *) new_node)->next = ((dlq_hdrT *) cur_node)->next;
    ((dlq_hdrT *) cur_node)->next = NULL;

    /* mark the new node as being in a Q */
    ((dlq_hdrT *) new_node)->hdr_typ = DLQ_DATA_NODE;

    /* mark the current node as removed from the Q */
    ((dlq_hdrT *) cur_node)->hdr_typ = DLQ_DEL_NODE;

    EXIT_CRIT;
    
}   /* END fuction dlq_swap */



#if defined(CPP_NO_MACROS)
/********************************************************************
*								    *
*		       function dlq_firstEntry			    *
*								    *
*********************************************************************/
void *dlq_firstEntry (const dlq_hdrT *	  listP)
{
    void    *retP;
    
#ifdef CPP_ICHK
    if (listP==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return NULL;
    }
    else if (!_hdr_node(listP))
    {
	err_msg(ERR_QNODE_NOT_HDR);
	return NULL;
    }
#endif
    ENTER_CRIT;
    retP = (listP != listP->next) ? listP->next : NULL;
    EXIT_CRIT;

    return retP;

}    /* END function dlq_firstEntry */
#endif	    /* CPP_NO_MACROS */



#if defined(CPP_NO_MACROS)
/********************************************************************
*								    *
*			function dlq_lastEntry			    *
*								    *
*********************************************************************/
void *dlq_lastEntry (const dlq_hdrT *	 listP)
{
    void    *retP;
    
#ifdef CPP_ICHK
    if (listP==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return NULL;
    }
    else if (!_hdr_node(listP))
    {
	err_msg(ERR_QNODE_NOT_HDR);
	return NULL;
    }
#endif

    ENTER_CRIT;
    retP = (listP != listP->next) ? listP->prev : NULL;
    EXIT_CRIT;

    return retP;

}    /* END function dlq_lastEntry */
#endif	    /* CPP_NO_MACROS */


#if defined(CPP_NO_MACROS)
/********************************************************************
*								    *
*			function dlq_empty			    *
*								    *
*********************************************************************/
boolean dlq_empty (const dlq_hdrT *  listP)
{
    boolean ret;
    
#ifdef CPP_ICHK
    if (listP==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return TRUE;
    }
    if (!_hdr_node(listP))
    {
	err_msg(ERR_QNODE_NOT_HDR);
	return TRUE;
    }
#endif

    ENTER_CRIT;
    ret = (boolean) (listP==listP->next);
    EXIT_CRIT;

    return ret;

}    /* END function dlq_empty */
#endif



/********************************************************************
*								    *
*			function dlq_block_enque		    *
*								    *
*********************************************************************/
void	   dlq_block_enque (dlq_hdrT * srcP, dlq_hdrT * dstP)
{
    dlq_hdrT  *sf, *sl, *dl;
    
#ifdef CPP_ICHK
    if (srcP==NULL || dstP==NULL)
	return;
    if (!_hdr_node(srcP) || !_hdr_node(dstP))
    {
	err_msg(ERR_QNODE_NOT_HDR);
	return;
    }
#endif

    /* check simple case first */
    if (dlq_empty(srcP))
	return;		/* nothing to add to dst que */

    /* check next simple case--dst empty */
    if (dlq_empty(dstP))
    {
	ENTER_CRIT;
	/* copy srcP pointers to dstP */
	dstP->next = srcP->next;
	dstP->prev = srcP->prev;
	
	/* relink first and last data nodes */
	dstP->next->prev = dstP;
	dstP->prev->next = dstP;
	
	/* make src que empty */
	srcP->next = srcP->prev = srcP;
	EXIT_CRIT;
	return;
    }

    ENTER_CRIT;    
    /* else neither que is empty...move [sf..sl] after dl */
    sf = srcP->next;	    /* source first */
    sl = srcP->prev;	    /* source last */
    dl = dstP->prev;	    /* dst last */
    
    /* extend the dstQ */
    dl->next = sf;	    /* link dl --> sf */
    sf->prev = dl;	    /* link dl <-- sf */

    /* relink the new last data node in dstQ */
    dstP->prev = sl;	    /* link hdr.prev --> sl */
    sl->next = dstP;	    /* link hdr.prev <-- sl */

    /* make the srcQ empty */
    srcP->prev = srcP->next = srcP;
    EXIT_CRIT;
    
}   /* END dlq_block_enque */




/********************************************************************
*								    *
*			function dlq_insertAhead		    *
*								    *
*********************************************************************/
void	   dlq_block_insertAhead (dlq_hdrT * srcP, void *dstP)
{
    
    REG dlq_hdrT    *sf, *sl, *d1, *d2;
    
#ifdef CPP_ICHK
    if (srcP==NULL || dstP==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return;
    }
    if (!_hdr_node(srcP))
    {
	err_msg(ERR_QNODE_NOT_HDR);
	return;
    }

    if (!_data_node( ((dlq_hdrT *)dstP) ))
    {
	err_msg(ERR_QNODE_NOT_DATA);
	return;
    }
#endif

    
    /* check simple case first */
    if (dlq_empty(srcP))
	return;		/* nothing to add to dst que */

    ENTER_CRIT;
    /* source que is empty... */
    sf = srcP->next;		/* source-first */
    sl = srcP->prev;		/* source-last */
    
    d1 = ((dlq_hdrT *) dstP)->prev;	    /* dest-begin-insert */
    d2 = (dlq_hdrT *) dstP;		    /* dest-end-insert (dstP) */

    /* link src-list into the dst-list */
    d1->next = sf;
    sf->prev = d1;
    
    d2->prev = sl;
    sl->next = d2;

    /* make srcQ empty */
    srcP->prev = srcP->next = srcP;
    EXIT_CRIT;
    
}   /* END dlq_block_insertAhead */




/********************************************************************
*								    *
*			function dlq_block_insertAfter		    *
*								    *
*********************************************************************/
void	   dlq_block_insertAfter (dlq_hdrT * srcP, void *dstP)
{
    
    REG dlq_hdrT   *sf, *sl, *d1, *d2;
    
#ifdef CPP_ICHK
    if (srcP==NULL || dstP==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return;
    }
    if (!_hdr_node(srcP))
    {
	err_msg(ERR_QNODE_NOT_HDR);
	return;
    }

    if (!_data_node( ((dlq_hdrT *)dstP) ))
    {
	err_msg(ERR_QNODE_NOT_DATA);
	return;
    }
#endif

    
    /* check simple case first */
    if (dlq_empty(srcP))
	return;		/* nothing to add to dst que */

    ENTER_CRIT;
    /* source que is not empty... */
    sf = srcP->next;		/* source-first */
    sl = srcP->prev;		/* source-last */

    /* make new chain: d1 + sf [ + ... ] + sl + d2 (+...) */
    d1 = (dlq_hdrT *) dstP;			/* dest-begin-insert */
    d2 = ((dlq_hdrT *) dstP)->next;		/* dest-end-insert (dstP) */

    /* link src-list into the dst-list */
    d1->next = sf;
    sf->prev = d1;
    
    d2->prev = sl;
    sl->next = d2;

    /* make srcQ empty */
    srcP->prev = srcP->next = srcP;
    EXIT_CRIT;
    
}   /* END dlq_block_insertAfter */




/********************************************************************
*								    *
*			function dlq_block_move			    *
*								    *
*********************************************************************/
void	   dlq_block_move (dlq_hdrT * srcQ, void *srcP, dlq_hdrT * dstQ)
{
    /* enque from [srcP ..  end of list] to the dstQ */
    
    REG dlq_hdrT   *sf, *sl;
    dlq_hdrT	    tmpQ;
    
#ifdef CPP_ICHK
    if (srcQ==NULL || srcP==NULL || dstQ==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return;
    }
    if (!_hdr_node(srcQ) || !_hdr_node(dstQ))
    {
	err_msg(ERR_QNODE_NOT_HDR);
	return;
    }
    if (!_data_node(srcP))
    {
	err_msg(ERR_QNODE_NOT_DATA);
	return;
    }
#endif	
    /* check simple case first */
    if (dlq_empty(srcQ))
	return;		/* nothing to add to dst que */

    ENTER_CRIT;    
    /* unlink the srcQ list from srcP to the end */
    sf = (dlq_hdrT *) srcP;
    sf->prev->next = srcQ;	
    sl = srcQ->prev;
    srcQ->prev = sf->prev;

    /* insert new chain into tmpQ */
    dlq_createSQue(&tmpQ);  
    tmpQ.next = sf;	
    sf->prev = &tmpQ;
    tmpQ.prev = sl;
    sl->next = &tmpQ;
    EXIT_CRIT;
    
    dlq_block_enque(&tmpQ, dstQ);
    
}   /* END dlq_block_move */



/********************************************************************
*								    *
*			function dlq_count 		            *
*								    *
*********************************************************************/
unsigned int  dlq_count (const dlq_hdrT *listP)
{
    REG const dlq_hdrT    *p;
    REG unsigned int cnt;
    
#ifdef CPP_ICHK
    if (listP==NULL)
    {
	err_msg(ERR_INTERNAL_PTR);
	return;
    }
    if (!_hdr_node(listP))
    {
	err_msg(ERR_QNODE_NOT_HDR);
	return;
    }
#endif

    cnt = 0;

    for (p = dlq_firstEntry(listP);
	 p != NULL;
	 p = dlq_nextEntry(p)) {
	cnt++;
    }

    return cnt;
    
}   /* END dlq_count */


/* END file dlq.c */
