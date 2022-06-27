
/*****************************************************************************************************
 * Copyright (C) 2017-2022 by Intrising
 *  - ian0113@intrising.com.tw
 * 
 * Generated by protoc-gen-yang@gen-yang
 * 
 *****************************************************************************************************/

#include <libxml/xmlstring.h>
#include "agt.h"
#include "agt_cb.h"
#include "agt_rpc.h"
#include "agt_timer.h"
#include "agt_util.h"
#include "dlq.h"
#include "ncx.h"
#include "ncx_feature.h"
#include "ncxmod.h"
#include "ncxtypes.h"
#include "procdefs.h"
#include "rpc.h"
#include "ses.h"
#include "status.h"
#include "val.h"
#include "val_util.h"
#include "xml_util.h"

#include "intri-sfp@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/sfp/intri-sfp-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/event/intri-event-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_sfp_mod;
static obj_template_t *intri_sfp_GetConfig_obj;
static obj_template_t *intri_sfp_SetConfig_obj;
static obj_template_t *intri_sfp_GetStatus_obj;

static status_t intri_sfp_SFP_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct sfppb_Config out;
  sfp_SFP_GetConfig(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_sfp_Config(
      resval,
    &out);
  return res;
}

static status_t intri_sfp_SFP_SetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct sfppb_Config in;
  struct emptypb_Empty out;
  sfp_SFP_SetConfig(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_sfp_SFP_GetStatus_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct sfppb_Info out;
  sfp_SFP_GetStatus(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_sfp_Info(
      resval,
    &out);
  return res;
}

status_t y_intri_sfp_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_sfp)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }

  if (revision && xml_strcmp(revision, y_R_intri_sfp)) {
    return ERR_NCX_WRONG_VERSION;
  }

  res = ncxmod_load_module(
      y_M_intri_sfp,
      y_R_intri_sfp,
      &agt_profile->agt_savedevQ,
      &intri_sfp_mod);
  if (res != NO_ERR) {
    return res;
  }

  intri_sfp_GetConfig_obj = ncx_find_object(
      intri_sfp_mod,
      "intri-sfp-GetConfig");
  if (intri_sfp_GetConfig_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_sfp,
      "intri-sfp-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_sfp_SFP_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_sfp_SetConfig_obj = ncx_find_object(
      intri_sfp_mod,
      "intri-sfp-SetConfig");
  if (intri_sfp_SetConfig_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_sfp,
      "intri-sfp-SetConfig",
      AGT_RPC_PH_INVOKE,
      intri_sfp_SFP_SetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_sfp_GetStatus_obj = ncx_find_object(
      intri_sfp_mod,
      "intri-sfp-GetStatus");
  if (intri_sfp_GetStatus_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_sfp,
      "intri-sfp-GetStatus",
      AGT_RPC_PH_INVOKE,
      intri_sfp_SFP_GetStatus_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

void y_intri_sfp_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_sfp,
      "intri-sfp-GetConfig");

  agt_rpc_unregister_method(
      y_M_intri_sfp,
      "intri-sfp-SetConfig");

  agt_rpc_unregister_method(
      y_M_intri_sfp,
      "intri-sfp-GetStatus");

}

