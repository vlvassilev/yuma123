// Code generated by protoc-gen-yang(*.c) DO NOT EDIT.

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

#include "sercom-stormcontrol@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../../realize/github.com/Intrising/intri-type/common/intri-common@2022-06-24.h"
#include "../../../../../../realize/github.com/Intrising/intri-type/device/intri-device@2022-06-24.h"
#include "../../../../../../realize/github.com/golang/protobuf/ptypes/empty/intri-empty@2022-06-24.h"

#include "../../../../../../genc-trans/github.com/Intrising/intri-type/common/intri-common-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/stormcontrol/intri-stormcontrol-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_stormcontrol_mod;

static status_t intri_stormcontrol_StormControl_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct stormcontrolpb_Config *out = malloc(sizeof(*out));

  stormcontrol_StormControl_GetConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_stormcontrol,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_stormcontrol_Config(outval, out);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }

  dlq_block_enque(&outval->v.childQ, &msg->rpc_dataQ);

  /* debug: print `val_value_t` in `msg->rpc_dataQ` */
  // for (val_value_t *val = (val_value_t *)dlq_firstEntry(&msg->rpc_dataQ);
  //      val != NULL;
  //      val = (val_value_t *)dlq_nextEntry(val)) {
  //   val_dump_value(val, 2);
  // }

  free(in);
  free(out);
  return res;
}
static status_t intri_stormcontrol_StormControl_SetEnabled_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct commonpb_Enabled *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  stormcontrol_StormControl_GetEnabled(out, in);
  res = build_to_priv_common_Enabled(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  stormcontrol_StormControl_SetEnabled(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_stormcontrol_StormControl_GetEnabled_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct commonpb_Enabled *out = malloc(sizeof(*out));

  stormcontrol_StormControl_GetEnabled(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_stormcontrol,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_common_Enabled(outval, out);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }

  dlq_block_enque(&outval->v.childQ, &msg->rpc_dataQ);

  /* debug: print `val_value_t` in `msg->rpc_dataQ` */
  // for (val_value_t *val = (val_value_t *)dlq_firstEntry(&msg->rpc_dataQ);
  //      val != NULL;
  //      val = (val_value_t *)dlq_nextEntry(val)) {
  //   val_dump_value(val, 2);
  // }

  free(in);
  free(out);
  return res;
}
static status_t intri_stormcontrol_StormControl_GetPortConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct stormcontrolpb_PortConfig *out = malloc(sizeof(*out));

  stormcontrol_StormControl_GetPortConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_stormcontrol,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_stormcontrol_PortConfig(outval, out);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }

  dlq_block_enque(&outval->v.childQ, &msg->rpc_dataQ);

  /* debug: print `val_value_t` in `msg->rpc_dataQ` */
  // for (val_value_t *val = (val_value_t *)dlq_firstEntry(&msg->rpc_dataQ);
  //      val != NULL;
  //      val = (val_value_t *)dlq_nextEntry(val)) {
  //   val_dump_value(val, 2);
  // }

  free(in);
  free(out);
  return res;
}
static status_t intri_stormcontrol_StormControl_UpdatePortConfigEntry_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct stormcontrolpb_PortConfigEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_stormcontrol_PortConfigEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  stormcontrol_StormControl_UpdatePortConfigEntry(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_stormcontrol_StormControl_UpdatePortConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct stormcontrolpb_PortConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  stormcontrol_StormControl_GetPortConfig(out, in);
  res = build_to_priv_stormcontrol_PortConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  stormcontrol_StormControl_UpdatePortConfig(in, out);

  free(in);
  free(out);
  return res;
}

status_t y_sercom_stormcontrol_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_stormcontrol)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }
  if (revision && xml_strcmp(revision, y_R_intri_stormcontrol)) {
    return ERR_NCX_WRONG_VERSION;
  }

  res = agt_load_sil_code(
    y_M_intri_device,
    y_R_intri_device,
    true);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_load_sil_code(
    y_M_intri_empty,
    y_R_intri_empty,
    true);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_load_sil_code(
    y_M_intri_common,
    y_R_intri_common,
    true);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = ncxmod_load_module(
      y_M_intri_stormcontrol,
      y_R_intri_stormcontrol,
      &agt_profile->agt_savedevQ,
      &intri_stormcontrol_mod);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_stormcontrol,
      "sercom-stormcontrol-StormControl-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_stormcontrol_StormControl_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_stormcontrol,
      "sercom-stormcontrol-StormControl-SetEnabled",
      AGT_RPC_PH_INVOKE,
      intri_stormcontrol_StormControl_SetEnabled_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_stormcontrol,
      "sercom-stormcontrol-StormControl-GetEnabled",
      AGT_RPC_PH_INVOKE,
      intri_stormcontrol_StormControl_GetEnabled_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_stormcontrol,
      "sercom-stormcontrol-StormControl-GetPortConfig",
      AGT_RPC_PH_INVOKE,
      intri_stormcontrol_StormControl_GetPortConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_stormcontrol,
      "sercom-stormcontrol-StormControl-UpdatePortConfigEntry",
      AGT_RPC_PH_INVOKE,
      intri_stormcontrol_StormControl_UpdatePortConfigEntry_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_stormcontrol,
      "sercom-stormcontrol-StormControl-UpdatePortConfig",
      AGT_RPC_PH_INVOKE,
      intri_stormcontrol_StormControl_UpdatePortConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

status_t y_sercom_stormcontrol_init2(void) {
  status_t res = NO_ERR;
  return res;
}

void y_sercom_stormcontrol_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_stormcontrol,
      "sercom-stormcontrol-StormControl-GetConfig");
  agt_rpc_unregister_method(
      y_M_intri_stormcontrol,
      "sercom-stormcontrol-StormControl-SetEnabled");
  agt_rpc_unregister_method(
      y_M_intri_stormcontrol,
      "sercom-stormcontrol-StormControl-GetEnabled");
  agt_rpc_unregister_method(
      y_M_intri_stormcontrol,
      "sercom-stormcontrol-StormControl-GetPortConfig");
  agt_rpc_unregister_method(
      y_M_intri_stormcontrol,
      "sercom-stormcontrol-StormControl-UpdatePortConfigEntry");
  agt_rpc_unregister_method(
      y_M_intri_stormcontrol,
      "sercom-stormcontrol-StormControl-UpdatePortConfig");
}
