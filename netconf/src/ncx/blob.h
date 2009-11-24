/*
 * Copyright (c) 2009, Netconf Central, Inc.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
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
