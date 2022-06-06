
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

*** Generated by yangdump 2.13-0

  Combined SIL module
  module intri-device
  revision 2022-05-24
  namespace http://intri.com.tw
  organization Intrising Inc.

 */

#include <libxml/xmlstring.h>

#include "procdefs.h"
#include "agt.h"
#include "agt_cb.h"
#include "agt_timer.h"
#include "agt_util.h"
#include "dlq.h"
#include "ncx.h"
#include "ncx_feature.h"
#include "ncxmod.h"
#include "ncxtypes.h"
#include "status.h"
#include "intri-device.h"
#include "../.libintrishare/libintrishare.h"

/* module static variables */
static ncx_module_t *intri_device_mod;
static obj_template_t *api_gets_obj;

/* put your static variables here */

/********************************************************************
 * FUNCTION y_intri_device_init_static_vars
 *
 * initialize module static variables
 *
 ********************************************************************/
static void y_intri_device_init_static_vars(void)
{
  intri_device_mod = NULL;
  api_gets_obj = NULL;

  /* init your static variables here */

} /* y_intri_device_init_static_vars */

/********************************************************************
 * FUNCTION intri_device_api_gets_test_feat_counter_get
 *
 * Get database object callback
 * Path: /api-gets/test-feat/counter
 * Fill in 'dstval' contents
 *
 * INPUTS:
 *     see ncx/getcb.h for details
 *
 * RETURNS:
 *     error status
 ********************************************************************/
static status_t intri_device_api_gets_test_feat_counter_get(
    ses_cb_t *scb,
    getcb_mode_t cbmode,
    const val_value_t *virval,
    val_value_t *dstval)
{
  status_t res = NO_ERR;
  uint64 counter;

  if (LOGDEBUG)
  {
    log_debug("\nEnter intri_device_api_gets_test_feat_counter_get callback");
  }

  /* remove the next line if scb is used */
  (void)scb;

  /* remove the next line if virval is used */
  (void)virval;

  if (cbmode != GETCB_GET_VALUE)
  {
    return ERR_NCX_OPERATION_NOT_SUPPORTED;
  }

  /* set the counter var here, change zero */
  counter = 0;
  VAL_ULONG(dstval) = counter;

  return res;

} /* intri_device_api_gets_test_feat_counter_get */

/********************************************************************
 * FUNCTION intri_device_api_gets_test_feat_mro
 *
 * Make read-only child nodes
 * Path: /api-gets/test-feat
 *
 * INPUTS:
 *     parentval == the parent struct to use for new child nodes
 *
 * RETURNS:
 *     error status
 ********************************************************************/
static status_t
intri_device_api_gets_test_feat_mro(val_value_t *parentval)
{
  status_t res = NO_ERR;
  val_value_t *childval = NULL;

  /* add /api-gets/test-feat/counter */
  childval = agt_make_virtual_leaf(
      parentval->obj,
      y_intri_device_N_counter,
      intri_device_api_gets_test_feat_counter_get,
      &res);
  if (childval != NULL)
  {
    val_add_child(childval, parentval);
  }
  else
  {
    return res;
  }

  return res;

} /* intri_device_api_gets_test_feat_mro */

/********************************************************************
 * FUNCTION intri_device_api_gets_test_mac_addr_mac_addr_get
 *
 * Get database object callback
 * Path: /api-gets/test-mac-addr/mac-addr
 * Fill in 'dstval' contents
 *
 * INPUTS:
 *     see ncx/getcb.h for details
 *
 * RETURNS:
 *     error status
 ********************************************************************/
static status_t intri_device_api_gets_test_mac_addr_mac_addr_get(
    ses_cb_t *scb,
    getcb_mode_t cbmode,
    const val_value_t *virval,
    val_value_t *dstval)
{
  status_t res = NO_ERR;
  const xmlChar *mac_addr;

  if (LOGDEBUG)
  {
    log_debug("\nEnter intri_device_api_gets_test_mac_addr_mac_addr_get callback");
  }

  /* remove the next line if scb is used */
  (void)scb;

  /* remove the next line if virval is used */
  (void)virval;

  if (cbmode != GETCB_GET_VALUE)
  {
    return ERR_NCX_OPERATION_NOT_SUPPORTED;
  }

  /* set the mac_addr var here, change EMPTY_STRING */
  GoString in = {};
  GoString out = {};
  GoInt err_code = 0;

  char in_char_arr[] = "{}";
  in.p = in_char_arr;
  in.n = sizeof(in_char_arr);
  Device_Device_GetMACAddress(&in, &out, &err_code);

  mac_addr = out.p;
  res = val_set_simval_obj(
      dstval,
      dstval->obj,
      mac_addr);

  return res;

} /* intri_device_api_gets_test_mac_addr_mac_addr_get */

/********************************************************************
 * FUNCTION intri_device_api_gets_test_mac_addr_mro
 *
 * Make read-only child nodes
 * Path: /api-gets/test-mac-addr
 *
 * INPUTS:
 *     parentval == the parent struct to use for new child nodes
 *
 * RETURNS:
 *     error status
 ********************************************************************/
static status_t
intri_device_api_gets_test_mac_addr_mro(val_value_t *parentval)
{
  status_t res = NO_ERR;
  val_value_t *childval = NULL;

  /* add /api-gets/test-mac-addr/mac-addr */
  childval = agt_make_virtual_leaf(
      parentval->obj,
      y_intri_device_N_mac_addr,
      intri_device_api_gets_test_mac_addr_mac_addr_get,
      &res);
  if (childval != NULL)
  {
    val_add_child(childval, parentval);
  }
  else
  {
    return res;
  }

  return res;

} /* intri_device_api_gets_test_mac_addr_mro */

/********************************************************************
 * FUNCTION intri_device_api_gets_mro
 *
 * Make read-only top-level node
 * Path: /api-gets
 *
 * RETURNS:
 *     error status
 ********************************************************************/
static status_t
intri_device_api_gets_mro(void)
{
  val_value_t *parentval = NULL, *childval = NULL;
  status_t res = NO_ERR;

  /* add /api-gets */
  res = agt_add_top_container(api_gets_obj, &parentval);
  if (res != NO_ERR)
  {
    return res;
  }
  res = agt_add_container(
      y_intri_device_M_intri_device,
      y_intri_device_N_test_feat,
      parentval,
      &childval);
  if (res != NO_ERR)
  {
    return res;
  }

  res = intri_device_api_gets_test_feat_mro(childval);
  if (res != NO_ERR)
  {
    return res;
  }

  res = agt_add_container(
      y_intri_device_M_intri_device,
      y_intri_device_N_test_mac_addr,
      parentval,
      &childval);
  if (res != NO_ERR)
  {
    return res;
  }

  res = intri_device_api_gets_test_mac_addr_mro(childval);
  if (res != NO_ERR)
  {
    return res;
  }

  return res;

} /* intri_device_api_gets_mro */

/********************************************************************
 * FUNCTION y_intri_device_init
 *
 * initialize the intri-device server instrumentation library
 *
 * INPUTS:
 *    modname == requested module name
 *    revision == requested version (NULL for any)
 *
 * RETURNS:
 *     error status
 ********************************************************************/
status_t y_intri_device_init(
    const xmlChar *modname,
    const xmlChar *revision)
{
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  y_intri_device_init_static_vars();

  /* change if custom handling done */
  if (xml_strcmp(modname, y_intri_device_M_intri_device))
  {
    return ERR_NCX_UNKNOWN_MODULE;
  }

  if (revision && xml_strcmp(revision, y_intri_device_R_intri_device))
  {
    return ERR_NCX_WRONG_VERSION;
  }
  res = ncxmod_load_module(
      y_intri_device_M_intri_device,
      y_intri_device_R_intri_device,
      &agt_profile->agt_savedevQ,
      &intri_device_mod);
  if (res != NO_ERR)
  {
    return res;
  }

  api_gets_obj = ncx_find_object(
      intri_device_mod,
      y_intri_device_N_api_gets);
  if (intri_device_mod == NULL)
  {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }
  /* put your module initialization code here */

  return res;
} /* y_intri_device_init */

/********************************************************************
 * FUNCTION y_intri_device_init2
 *
 * SIL init phase 2: non-config data structures
 * Called after running config is loaded
 *
 * RETURNS:
 *     error status
 ********************************************************************/
status_t y_intri_device_init2(void)
{
  status_t res = NO_ERR;

  res = intri_device_api_gets_mro();
  if (res != NO_ERR)
  {
    return res;
  }

  /* put your init2 code here */

  return res;
} /* y_intri_device_init2 */

/********************************************************************
 * FUNCTION y_intri_device_cleanup
 *    cleanup the server instrumentation library
 *
 ********************************************************************/
void y_intri_device_cleanup(void)
{
  /* put your cleanup code here */

} /* y_intri_device_cleanup */

/* END intri_device.c */
