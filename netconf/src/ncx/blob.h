#ifndef _H_blob
#define _H_blob
/*  FILE: blob.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    binary to string conversion for database storage

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
22-apr-05    abb      begun
*/

extern void 
    blob2bin (const char *pblob, 
	      unsigned char *pbuff,
	      uint32 bsize);

extern void 
    bin2blob (const unsigned char *pbuff,
	      char  *pblob,
	      uint32 bsize);

#endif	    /* _H_blob */
