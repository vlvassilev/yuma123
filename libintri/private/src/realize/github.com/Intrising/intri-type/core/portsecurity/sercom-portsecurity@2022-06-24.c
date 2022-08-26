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

#include "sercom-portsecurity@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../../realize/github.com/Intrising/intri-type/device/intri-device@2022-06-24.h"
#include "../../../../../../realize/github.com/golang/protobuf/ptypes/empty/intri-empty@2022-06-24.h"

#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/portsecurity/intri-portsecurity-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_portsecurity_mod;

static status_t intri_portsecurity_PortSecurity_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct portsecuritypb_Config *out = malloc(sizeof(*out));

  portsecurity_PortSecurity_GetConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_portsecurity,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_portsecurity_Config(outval, out);
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
static status_t intri_portsecurity_PortSecurity_UpdatePortSecurityConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct portsecuritypb_PortSecurityConfigEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_portsecurity_PortSecurityConfigEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  portsecurity_PortSecurity_UpdatePortSecurityConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_portsecurity_PortSecurity_AddPortSecureEntry_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct portsecuritypb_PortSecureEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_portsecurity_PortSecureEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  portsecurity_PortSecurity_AddPortSecureEntry(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_portsecurity_PortSecurity_DeletePortSecureEntry_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct portsecuritypb_PortSecureEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_portsecurity_PortSecureEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  portsecurity_PortSecurity_DeletePortSecureEntry(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_portsecurity_PortSecurity_GetPortSecurityInfo_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct portsecuritypb_Status *out = malloc(sizeof(*out));

  portsecurity_PortSecurity_GetPortSecurityInfo(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_portsecurity,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_portsecurity_Status(outval, out);
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

status_t y_sercom_portsecurity_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_portsecurity)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }
  if (revision && xml_strcmp(revision, y_R_intri_portsecurity)) {
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

  res = ncxmod_load_module(
      y_M_intri_portsecurity,
      y_R_intri_portsecurity,
      &agt_profile->agt_savedevQ,
      &intri_portsecurity_mod);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portsecurity,
      "sercom-portsecurity-PortSecurity-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_portsecurity_PortSecurity_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portsecurity,
      "sercom-portsecurity-PortSecurity-UpdatePortSecurityConfig",
      AGT_RPC_PH_INVOKE,
      intri_portsecurity_PortSecurity_UpdatePortSecurityConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portsecurity,
      "sercom-portsecurity-PortSecurity-AddPortSecureEntry",
      AGT_RPC_PH_INVOKE,
      intri_portsecurity_PortSecurity_AddPortSecureEntry_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portsecurity,
      "sercom-portsecurity-PortSecurity-DeletePortSecureEntry",
      AGT_RPC_PH_INVOKE,
      intri_portsecurity_PortSecurity_DeletePortSecureEntry_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portsecurity,
      "sercom-portsecurity-PortSecurity-GetPortSecurityInfo",
      AGT_RPC_PH_INVOKE,
      intri_portsecurity_PortSecurity_GetPortSecurityInfo_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

status_t y_sercom_portsecurity_init2(void) {
  status_t res = NO_ERR;
  return res;
}

void y_sercom_portsecurity_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_portsecurity,
      "sercom-portsecurity-PortSecurity-GetConfig");
  agt_rpc_unregister_method(
      y_M_intri_portsecurity,
      "sercom-portsecurity-PortSecurity-UpdatePortSecurityConfig");
  agt_rpc_unregister_method(
      y_M_intri_portsecurity,
      "sercom-portsecurity-PortSecurity-AddPortSecureEntry");
  agt_rpc_unregister_method(
      y_M_intri_portsecurity,
      "sercom-portsecurity-PortSecurity-DeletePortSecureEntry");
  agt_rpc_unregister_method(
      y_M_intri_portsecurity,
      "sercom-portsecurity-PortSecurity-GetPortSecurityInfo");
}
