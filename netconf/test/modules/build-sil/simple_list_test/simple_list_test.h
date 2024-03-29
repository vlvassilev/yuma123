
#ifndef _H_simple_list_test
#define _H_simple_list_test
/* 
 * Copyright (c) 2008-2012, Andy Bierman, All Rights Reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *

*** Generated by yangdump 2.2.1732

    Combined SIL header
    module simple_list_test
    revision 2008-11-20
    namespace http://netconfcentral.org/ns/simple_list_test

 */

#include "dlq.h"
#include "ncxtypes.h"
#include "op.h"
#include "status.h"
#include "val.h"

#ifdef __cplusplus
extern "C" {
#endif

#define y_simple_list_test_M_simple_list_test (const xmlChar *)"simple_list_test"
#define y_simple_list_test_R_simple_list_test (const xmlChar *)"2008-11-20"

#define y_simple_list_test_N_count (const xmlChar *)"count"
#define y_simple_list_test_N_get_counter (const xmlChar *)"get-counter"
#define y_simple_list_test_N_inc_counter (const xmlChar *)"inc-counter"
#define y_simple_list_test_N_simple_list (const xmlChar *)"simple_list"
#define y_simple_list_test_N_theKey (const xmlChar *)"theKey"
#define y_simple_list_test_N_theList (const xmlChar *)"theList"
#define y_simple_list_test_N_theVal (const xmlChar *)"theVal"

/* list /simple_list/theList */
typedef struct y_simple_list_test_T_simple_list_theList_ {
    dlq_hdr_t qhdr;
    xmlChar *theKey;
    xmlChar *theVal;
} y_simple_list_test_T_simple_list_theList;

/* container /simple_list */
typedef struct y_simple_list_test_T_simple_list_ {
    dlq_hdr_t theList;
} y_simple_list_test_T_simple_list;

/* container /inc-counter/input */
typedef struct y_simple_list_test_T_inc_counter_input_ {
} y_simple_list_test_T_inc_counter_input;

/* container /inc-counter/output */
typedef struct y_simple_list_test_T_inc_counter_output_ {
} y_simple_list_test_T_inc_counter_output;

/* rpc /inc-counter */
typedef struct y_simple_list_test_T_inc_counter_ {
    y_simple_list_test_T_inc_counter_input input;
    y_simple_list_test_T_inc_counter_output output;
} y_simple_list_test_T_inc_counter;

/* container /get-counter/output */
typedef struct y_simple_list_test_T_get_counter_output_ {
    uint32 count;
} y_simple_list_test_T_get_counter_output;

/* container /get-counter/input */
typedef struct y_simple_list_test_T_get_counter_input_ {
} y_simple_list_test_T_get_counter_input;

/* rpc /get-counter */
typedef struct y_simple_list_test_T_get_counter_ {
    y_simple_list_test_T_get_counter_output output;
    y_simple_list_test_T_get_counter_input input;
} y_simple_list_test_T_get_counter;
/********************************************************************
* FUNCTION y_simple_list_test_init
* 
* initialize the simple_list_test server instrumentation library
* 
* INPUTS:
*    modname == requested module name
*    revision == requested version (NULL for any)
* 
* RETURNS:
*     error status
********************************************************************/
extern status_t y_simple_list_test_init (
    const xmlChar *modname,
    const xmlChar *revision);

/********************************************************************
* FUNCTION y_simple_list_test_init2
* 
* SIL init phase 2: non-config data structures
* Called after running config is loaded
* 
* RETURNS:
*     error status
********************************************************************/
extern status_t y_simple_list_test_init2 (void);

/********************************************************************
* FUNCTION y_simple_list_test_cleanup
*    cleanup the server instrumentation library
* 
********************************************************************/
extern void y_simple_list_test_cleanup (void);

#ifdef __cplusplus
} /* end extern 'C' */
#endif

#endif
