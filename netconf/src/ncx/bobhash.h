/*

Public domain hash function from the 1997 Dr Dobbs article
By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

This function is referenced in - From Packet Sampling Techniques
RFC 5475 -  implemented from <draft-ietf-psamp-sample-tech-07.txt>

*/

typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;   /* unsigned 1-byte quantities */

#define hashsize(n) ((ub4)1<<(n))
#define hashmask(n) (hashsize(n)-1)

extern ub4 bobhash(register const ub1 *k,         /* the key */
                   register ub4  length,    /* the length of the key */
                   register ub4  initval);  /* the previous hash, or an arbitrary value */
