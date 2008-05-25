#ifndef _H_dlq
#define _H_dlq
/*  FILE: dlq.h
*********************************************************************
*                                    *
*             P U R P O S E                    *
*                                    *
*********************************************************************

    dlq provides general double-linked list and queue support:
   
    API Functions
    =============

       QUEUE initialization/cleanup
       * dlq_createQue - create and initialize dynamic queue hdr
       * dlq_destroyQue - destroy previously created dynamic queue
       * dlq_createSQue - initialize static queue hdr, no destroy needed

       FIFO queue operations
       * dlq_enque - add node to end of list
       * dlq_block_enque - add N nodes to end of list
       * dlq_deque - return first node, remove from list

       TEST queue operations
       * dlq_empty - return TRUE if queue is empty, FALSE otherwise

       LINEAR search (linked list)
       * dlq_nextEntry - return node AFTER param node - leave in list
       * dlq_prevEntry - return node BEFORE param node - leave in list
       * dlq_firstEntry - return first node in the Q
       * dlq_lastEntry - return last node in the Q

       Q INSERTION operations
       * dlq_insertAhead - add node in list ahead of param node
       * dlq_insertAfter - add node in list after param node
       * dlq_block_insertAhead - add N nodes in list ahead of param node
       * dlq_block_insertAfter - add N nodes in list after param node
       * dlq_block_move - move contents of srcQ to the destintaion Q

       Q DELETION operations
       * dlq_remove - remove a node from a linked list

       Q DEBUG operations
       * dlq_dumpHdr - printf Q header info  (CPP_DEBUG required)

   CPP Macros Used:
   ================

       CPP_DEBUG - enables debug code

       CPP_NO_MACROS - forces function calls instead of macros for
           some functions.  Should not be used except in some debug modes

       CPP_ICHK - this will force function calls instead of macros and
           enable lots of parameter checking (internal checks). Should
           only be used for unit test debugging

       ENTER_CRIT - this macro hook can be set to call an enter critical
           section function for thread-safe queueing
       EXIT_CRIT - this macro hook can be set to call an exit critical
           section function for thread-safe queueing

*********************************************************************
*                                    *
*           C H A N G E     H I S T O R Y                *
*                                    *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
06-jan-89    abb      Begun.
18-jan-91    abb      adapted for depend project
12-mar-91    abb      adapted for DAVID sbee project
14-jun-91    abb      changed que.h to dlq.h
27-apr-05    abb      update docs and use for netconf project
15-feb-06    abb      make DLQ module const compatible
                      get rid of dlq_hdrPT, as this doesn't work 
                      for const pointers.
15-sep-06    abb      added dlq_swap function
26-jan-07    abb      added dlq_hdr_t alias for NCX naming conventions
12-oct-07    abb      add dlq_count
*/


/********************************************************************
*
*                             C O N S T A N T S
*
*********************************************************************/

/* que header types */
#define        DLQ_NULL_NODE    1313
#define        DLQ_SHDR_NODE    2727
#define        DLQ_DHDR_NODE    3434
#define        DLQ_DATA_NODE    5757
#define        DLQ_DEL_NODE     8686
#define        DLQ_DEL_DHDR     9696

#define dlq_hdr_t  dlq_hdrT


/********************************************************************
*
*                           T Y P E S
*                                    
*********************************************************************/

typedef struct TAGdlq_hdrT 
{
     unsigned short        hdr_typ;
     struct TAGdlq_hdrT    *prev;
     struct TAGdlq_hdrT    *next;
} dlq_hdrT;


/* shorthand macros */
#define _hdr_node(P) (((const dlq_hdrT *)(P))->hdr_typ==DLQ_SHDR_NODE \
    || ((const dlq_hdrT *)(P))->hdr_typ==DLQ_DHDR_NODE)

#define _data_node(P) (((const dlq_hdrT *)(P))->hdr_typ==DLQ_DATA_NODE)

/********************************************************************
*
*                          F U N C T I O N S
*
*********************************************************************/
extern dlq_hdrT * dlq_createQue (void);

extern void dlq_destroyQue (dlq_hdrT * listP);

extern void dlq_createSQue (dlq_hdrT * queAddr);

extern void dlq_enque (void *newP, dlq_hdrT * listP);

extern void dlq_block_enque (dlq_hdrT * srcP, dlq_hdrT * dstP);

extern void *dlq_deque (dlq_hdrT * listP);

#if defined(CPP_NO_MACROS)
extern boolean dlq_empty (const dlq_hdrT * listP);
#else
#define dlq_empty(P) (boolean)((P)==((const dlq_hdrT *)(P))->next)
#endif        /* CPP_NO_MACROS */

#if defined(CPP_NO_MACROS)
extern void *dlq_nextEntry (const void *nodeP);
#else
#define dlq_nextEntry(P)  (_data_node(((const dlq_hdrT *) (P))->next) ? \
      ((const dlq_hdrT *) (P))->next : NULL)
#endif        /* END CPP_NO_MACROS */

#if defined(CPP_NO_MACROS)
extern void *dlq_prevEntry (const void *nodeP);
#else
#define dlq_prevEntry(P) (_data_node(((const dlq_hdrT *) (P))->prev ) ? \
      ((const dlq_hdrT *) (P))->prev : NULL)
#endif    /* CPP_NO_MACROS */

#if defined(CPP_NO_MACROS)
extern void *dlq_firstEntry (const dlq_hdrT * listP);
#else
#define dlq_firstEntry(P) ((P) != ((const dlq_hdrT *)(P))->next ? \
        ((const dlq_hdrT *)(P))->next : NULL)
#endif        /* CPP_NO_MACROS */

#define dlq_firstROEntry(P) ((P) != ((dlq_hdrT *)(P))->next ? \
        ((const dlq_hdrT *)(P))->next : NULL)

#define dlq_nextROEntry(P)  (_data_node(((dlq_hdrT *) (P))->next) ? \
      ((const dlq_hdrT *) (P))->next : NULL)

#if defined(CPP_NO_MACROS)
extern void *dlq_lastEntry (const dlq_hdrT * listP);
#else
#define dlq_lastEntry(P) ((P) != ((const dlq_hdrT *)(P))->next ? \
        ((const dlq_hdrT *)(P))->prev : NULL)
#endif        /* CPP_NO_MACROS */

extern void dlq_insertAhead (void *newP, void *nodeP);

extern void dlq_insertAfter (void *newP, void *nodeP);

extern void dlq_block_insertAhead (dlq_hdrT *srcP, void *dstP);

extern void dlq_block_insertAfter (dlq_hdrT *srcP, void *dstP);

extern void dlq_block_move (dlq_hdrT *srcQ, void *srcP, dlq_hdrT * dstQ);

extern void dlq_remove (void *nodeP);

extern void dlq_swap (void *new_node, void *cur_node);


#ifdef CPP_DEBUG
extern void dlq_dumpHdr (const void *nodeP);
#endif

extern unsigned int dlq_count (const dlq_hdrT *listP);

#endif    /* _H_dlq */
