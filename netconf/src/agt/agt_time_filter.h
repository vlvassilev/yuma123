
#ifndef _H_agt_time_filter
#define _H_agt_time_filter
/* 

 * Copyright (c) 2008 - 2012, Andy Bierman, All Rights Reserved.
 * All Rights Reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *

*** Originally generated by yangdump 2.0.1297

    module yuma-time-filter
    revision 2012-11-15

    namespace http://netconfcentral.org/ns/yuma-time-filter
    organization Netconf Central

 */

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define y_yuma_time_filter_M_yuma_time_filter \
    (const xmlChar *)"yuma-time-filter"
#define y_yuma_time_filter_R_yuma_time_filter (const xmlChar *)"2012-11-15"


/* yuma-time-filter module init 1 */
extern status_t
    y_yuma_time_filter_init (
        const xmlChar *modname,
        const xmlChar *revision);

/* yuma-time-filter module init 2 */
extern status_t
    y_yuma_time_filter_init2 (void);

/* yuma-time-filter module cleanup */
extern void
    y_yuma_time_filter_cleanup (void);

#ifdef __cplusplus
}  /* end extern 'C' */
#endif

#endif
