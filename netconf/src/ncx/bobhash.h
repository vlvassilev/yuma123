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
#ifndef _H_bobhash
#define _H_bobhash

/*  FILE: bobhash.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    BOB hash function

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
17-oct-05    abb      Begun -- copied from PSAMP Sampling Techniques 

*/

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

#define hashsize(n) ((uint32)1<<(n))  

#define hashmask(n) (hashsize(n)-1)  


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern uint32
bobhash (register const uint8 *k,        /* the key */  
         register uint32  length,   /* the length of the key */  
         register uint32  initval);  /* an arbitrary value */  


#endif	    /* _H_bobhash */
