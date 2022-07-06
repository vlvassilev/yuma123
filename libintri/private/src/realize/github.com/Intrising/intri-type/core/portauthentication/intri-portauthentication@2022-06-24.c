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

#include "intri-portauthentication@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../../genc-trans/github.com/Intrising/intri-type/common/intri-common-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/access/intri-access-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/portauthentication/intri-portauthentication-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_portauthentication_mod;

static status_t intri_portauthentication_PortAuthentication_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct portauthenticationpb_Config *out = malloc(sizeof(*out));

  portauthentication_PortAuthentication_GetConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_portauthentication,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_portauthentication_Config(outval, out);
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
static status_t intri_portauthentication_PortAuthentication_GetSystemConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct portauthenticationpb_SystemConfig *out = malloc(sizeof(*out));

  portauthentication_PortAuthentication_GetSystemConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_portauthentication,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_portauthentication_SystemConfig(outval, out);
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
static status_t intri_portauthentication_PortAuthentication_SetSystemConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct portauthenticationpb_SystemConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  portauthentication_PortAuthentication_GetSystemConfig(out, in);
  res = build_to_priv_portauthentication_SystemConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  portauthentication_PortAuthentication_SetSystemConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_portauthentication_PortAuthentication_GetPortConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct portauthenticationpb_PortConfig *out = malloc(sizeof(*out));

  portauthentication_PortAuthentication_GetPortConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_portauthentication,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_portauthentication_PortConfig(outval, out);
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
static status_t intri_portauthentication_PortAuthentication_UpdatePortConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct portauthenticationpb_PortConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  portauthentication_PortAuthentication_GetPortConfig(out, in);
  res = build_to_priv_portauthentication_PortConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  portauthentication_PortAuthentication_UpdatePortConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_portauthentication_PortAuthentication_GetAuthorizedMACsConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct portauthenticationpb_AuthorizedMACs *out = malloc(sizeof(*out));

  portauthentication_PortAuthentication_GetAuthorizedMACsConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_portauthentication,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_portauthentication_AuthorizedMACs(outval, out);
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
static status_t intri_portauthentication_PortAuthentication_AddAuthorizedMACsEntryConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct portauthenticationpb_AuthorizedMACsEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_portauthentication_AuthorizedMACsEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  portauthentication_PortAuthentication_AddAuthorizedMACsEntryConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_portauthentication_PortAuthentication_DeleteAuthorizedMACsEntryConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct commonpb_Name *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_common_Name(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  portauthentication_PortAuthentication_DeleteAuthorizedMACsEntryConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_portauthentication_PortAuthentication_UpdateAuthorizedMACsEntryConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct portauthenticationpb_AuthorizedMACsEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_portauthentication_AuthorizedMACsEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  portauthentication_PortAuthentication_UpdateAuthorizedMACsEntryConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_portauthentication_PortAuthentication_GetPortStatus_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct portauthenticationpb_PortStatus *out = malloc(sizeof(*out));

  portauthentication_PortAuthentication_GetPortStatus(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_portauthentication,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_portauthentication_PortStatus(outval, out);
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
static status_t intri_portauthentication_PortAuthentication_GetMACAuthorizationStatus_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct portauthenticationpb_PortAuthorizationStatus *out = malloc(sizeof(*out));

  portauthentication_PortAuthentication_GetMACAuthorizationStatus(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_portauthentication,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_portauthentication_PortAuthorizationStatus(outval, out);
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
static status_t intri_portauthentication_PortAuthentication_Get8021XAuthorizationStatus_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct portauthenticationpb_PortAuthorizationStatus *out = malloc(sizeof(*out));

  portauthentication_PortAuthentication_Get8021XAuthorizationStatus(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_portauthentication,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_portauthentication_PortAuthorizationStatus(outval, out);
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
static status_t intri_portauthentication_PortAuthentication_GetUserStatus_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct portauthenticationpb_UserStatus *out = malloc(sizeof(*out));

  portauthentication_PortAuthentication_GetUserStatus(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_portauthentication,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_portauthentication_UserStatus(outval, out);
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
static status_t intri_portauthentication_PortAuthentication_RunPortConfigLearnMACNow_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct portauthenticationpb_LearnMACNowEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_portauthentication_LearnMACNowEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  portauthentication_PortAuthentication_RunPortConfigLearnMACNow(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_portauthentication_PortAuthentication_RunPortConfigReauthenticate_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct devicepb_InterfaceIdentify *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_device_InterfaceIdentify(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  portauthentication_PortAuthentication_RunPortConfigReauthenticate(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_portauthentication_PortAuthentication_RunPortConfigUnauthorizeMAC_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct portauthenticationpb_UnauthorizeMACEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_portauthentication_UnauthorizeMACEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  portauthentication_PortAuthentication_RunPortConfigUnauthorizeMAC(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_portauthentication_PortAuthentication_SetAuthenticationServers_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_AuthenticationServersConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_access_AuthenticationServersConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  portauthentication_PortAuthentication_SetAuthenticationServers(in, out);

  free(in);
  free(out);
  return res;
}

status_t y_intri_portauthentication_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_portauthentication)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }
  if (revision && xml_strcmp(revision, y_R_intri_portauthentication)) {
    return ERR_NCX_WRONG_VERSION;
  }

  res = ncxmod_load_module(
      y_M_intri_portauthentication,
      y_R_intri_portauthentication,
      &agt_profile->agt_savedevQ,
      &intri_portauthentication_mod);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-GetSystemConfig",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_GetSystemConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-SetSystemConfig",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_SetSystemConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-GetPortConfig",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_GetPortConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-UpdatePortConfig",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_UpdatePortConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-GetAuthorizedMACsConfig",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_GetAuthorizedMACsConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-AddAuthorizedMACsEntryConfig",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_AddAuthorizedMACsEntryConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-DeleteAuthorizedMACsEntryConfig",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_DeleteAuthorizedMACsEntryConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-UpdateAuthorizedMACsEntryConfig",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_UpdateAuthorizedMACsEntryConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-GetPortStatus",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_GetPortStatus_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-GetMACAuthorizationStatus",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_GetMACAuthorizationStatus_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-Get8021XAuthorizationStatus",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_Get8021XAuthorizationStatus_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-GetUserStatus",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_GetUserStatus_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-RunPortConfigLearnMACNow",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_RunPortConfigLearnMACNow_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-RunPortConfigReauthenticate",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_RunPortConfigReauthenticate_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-RunPortConfigUnauthorizeMAC",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_RunPortConfigUnauthorizeMAC_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-SetAuthenticationServers",
      AGT_RPC_PH_INVOKE,
      intri_portauthentication_PortAuthentication_SetAuthenticationServers_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

status_t y_intri_portauthentication_init2(void) {
  status_t res = NO_ERR;
  return res;
}

void y_intri_portauthentication_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-GetConfig");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-GetSystemConfig");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-SetSystemConfig");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-GetPortConfig");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-UpdatePortConfig");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-GetAuthorizedMACsConfig");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-AddAuthorizedMACsEntryConfig");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-DeleteAuthorizedMACsEntryConfig");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-UpdateAuthorizedMACsEntryConfig");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-GetPortStatus");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-GetMACAuthorizationStatus");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-Get8021XAuthorizationStatus");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-GetUserStatus");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-RunPortConfigLearnMACNow");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-RunPortConfigReauthenticate");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-RunPortConfigUnauthorizeMAC");
  agt_rpc_unregister_method(
      y_M_intri_portauthentication,
      "intri-portauthentication-PortAuthentication-SetAuthenticationServers");
}
