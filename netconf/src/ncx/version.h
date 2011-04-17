/*
 * Copyright (c) 2009, 2010, Andy Bierman
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
#ifndef _H_version
#define _H_version
/*  FILE: version.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

    Contains the current Yuma version ID

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
23-jun-09    abb      Begun.
01-jun-10    abb      Switched to 1.x instead of 0.x
                      to align with debian packaging standards
09-oct-10    abb      Bumped version to 1.14 
17-apr-11    abb      Bumped version to 1.15
*/

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/

#define YUMA_VERSION  (const xmlChar *)"1.15"

#ifdef __cplusplus
}  /* end extern 'C' */
#endif

#endif            /* _H_version */
