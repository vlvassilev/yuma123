/*
 * Copyright (c) 2009, 2010, Netconf Central, Inc.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
#ifndef _H_b64
#define _H_b64

/*  FILE: b64.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    RFC 4648 base64 support, from b64.c

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
25-oct-08    abb      Begun

*/

#ifndef _H_status
#include "status.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/


/*
 * b64_encode
 *
 * base64 encode a stream adding padding and line breaks as per spec.
 *
 * adapted from the b64 function for 2 FILE parameters
 * using internal buffers instead
 *
 * INPUTS:
 *   inbuff == pointer to buffer of binary chars
 *   inbufflen == number of binary chars in inbuff
 *   outbuff == pointer to the output buffer to use
 *              MAY BE NULL; just get encoded length in that case
 *   outbufflen == max number of chars to write to outbuff
 *   linesize == the output line length to use
 *   retlen == address of return length
 *   
 * OUTPUTS:
 *   *retlen == number of chars written to outbuff,
 *             or would have been written if it was non-NULL
 *
 * RETURNS:
 *   status:
 *     NO_ERR if all OK
 *     ERR_BUFF_OVFL if outbuff not big enough
 */
extern status_t
    b64_encode (const unsigned char *inbuff,
		unsigned int inbufflen,
		unsigned char *outbuff, 
		unsigned int outbufflen,
		unsigned int linesize,
		unsigned int *retlen);


/*
 * b64_decode
 *
 * decode a base64 encoded stream discarding padding, line breaks and noise
 *
 * INPUTS:
 *   inbuff == pointer to buffer of base64 chars
 *   inbufflen == number of chars in inbuff
 *   outbuff == pointer to the output buffer to use
 *              MAY BE NULL; just get encoded length in that case
 *   outbufflen == max number of chars to write to outbuff
 *   linesize == the output line length to use
 *   retlen == address of return length
 *   
 * OUTPUTS:
 *   *retlen == number of chars written to outbuff
 *
 * RETURNS:
 *   status:
 *     NO_ERR if all OK
 *     ERR_BUFF_OVFL if outbuff not big enough
 */
extern status_t
    b64_decode (const unsigned char *inbuff, 
		unsigned int inbufflen,
		unsigned char *outbuff,
		unsigned int outbufflen,
		unsigned int *retlen);

#ifdef __cplusplus
}  /* end extern 'C' */
#endif

#endif	    /* _H_b64 */
