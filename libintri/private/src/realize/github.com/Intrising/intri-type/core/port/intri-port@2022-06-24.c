
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

#include "intri-port@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/port/intri-port-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/stp/intri-stp-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/event/intri-event-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_port_mod;
static obj_template_t *intri_port_GetConfig_obj;
static obj_template_t *intri_port_UpdateAliasConfig_obj;
static obj_template_t *intri_port_UpdateOperationConfig_obj;
static obj_template_t *intri_port_UpdateSpeedDuplexConfig_obj;
static obj_template_t *intri_port_UpdateFlowControlConfig_obj;
static obj_template_t *intri_port_UpdateEnergyEfficiencyConfig_obj;
static obj_template_t *intri_port_GetStatus_obj;
static obj_template_t *intri_port_RunRestartPorts_obj;
static obj_template_t *intri_port_GetLgStatus_obj;
static obj_template_t *intri_port_RunEnablePort_obj;
static obj_template_t *intri_port_GetPortStatus_obj;
static obj_template_t *intri_port_GetLgPortStatus_obj;

static status_t intri_port_Port_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct portpb_Config out;
  port_Port_GetConfig(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_port_Config(
      resval,
    &out);
  return res;
}

static status_t intri_port_Port_UpdateAliasConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct portpb_AliasConfig in;
  struct emptypb_Empty out;
  port_Port_UpdateAliasConfig(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_port_Port_UpdateOperationConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct portpb_OperationConfig in;
  struct emptypb_Empty out;
  port_Port_UpdateOperationConfig(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_port_Port_UpdateSpeedDuplexConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct portpb_SpeedDuplexConfig in;
  struct emptypb_Empty out;
  port_Port_UpdateSpeedDuplexConfig(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_port_Port_UpdateFlowControlConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct portpb_FlowControlConfig in;
  struct emptypb_Empty out;
  port_Port_UpdateFlowControlConfig(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_port_Port_UpdateEnergyEfficiencyConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct portpb_EnergyEfficiencyConfig in;
  struct emptypb_Empty out;
  port_Port_UpdateEnergyEfficiencyConfig(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_port_Port_GetStatus_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct portpb_Status out;
  port_Port_GetStatus(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_port_Status(
      resval,
    &out);
  return res;
}

static status_t intri_port_Port_RunRestartPorts_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct devicepb_PortList in;
  struct emptypb_Empty out;
  port_Port_RunRestartPorts(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_port_Port_GetLgStatus_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct portpb_LgPortStatus out;
  port_Port_GetLgStatus(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_port_LgPortStatus(
      resval,
    &out);
  return res;
}

static status_t intri_port_Port_RunEnablePort_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct portpb_OperationEntry in;
  struct emptypb_Empty out;
  port_Port_RunEnablePort(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_port_Port_GetPortStatus_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct devicepb_InterfaceIdentify in;
  struct portpb_StatusEntry out;
  port_Port_GetPortStatus(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_port_StatusEntry(
      resval,
    &out);
  return res;
}

static status_t intri_port_Port_GetLgPortStatus_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct devicepb_InterfaceIdentify in;
  struct portpb_LgPortStatusEntry out;
  port_Port_GetLgPortStatus(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_port_LgPortStatusEntry(
      resval,
    &out);
  return res;
}

status_t y_intri_port_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_port)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }

  if (revision && xml_strcmp(revision, y_R_intri_port)) {
    return ERR_NCX_WRONG_VERSION;
  }

  res = ncxmod_load_module(
      y_M_intri_port,
      y_R_intri_port,
      &agt_profile->agt_savedevQ,
      &intri_port_mod);
  if (res != NO_ERR) {
    return res;
  }

  intri_port_GetConfig_obj = ncx_find_object(
      intri_port_mod,
      "intri-port-GetConfig");
  if (intri_port_GetConfig_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_port,
      "intri-port-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_port_Port_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_port_UpdateAliasConfig_obj = ncx_find_object(
      intri_port_mod,
      "intri-port-UpdateAliasConfig");
  if (intri_port_UpdateAliasConfig_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_port,
      "intri-port-UpdateAliasConfig",
      AGT_RPC_PH_INVOKE,
      intri_port_Port_UpdateAliasConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_port_UpdateOperationConfig_obj = ncx_find_object(
      intri_port_mod,
      "intri-port-UpdateOperationConfig");
  if (intri_port_UpdateOperationConfig_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_port,
      "intri-port-UpdateOperationConfig",
      AGT_RPC_PH_INVOKE,
      intri_port_Port_UpdateOperationConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_port_UpdateSpeedDuplexConfig_obj = ncx_find_object(
      intri_port_mod,
      "intri-port-UpdateSpeedDuplexConfig");
  if (intri_port_UpdateSpeedDuplexConfig_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_port,
      "intri-port-UpdateSpeedDuplexConfig",
      AGT_RPC_PH_INVOKE,
      intri_port_Port_UpdateSpeedDuplexConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_port_UpdateFlowControlConfig_obj = ncx_find_object(
      intri_port_mod,
      "intri-port-UpdateFlowControlConfig");
  if (intri_port_UpdateFlowControlConfig_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_port,
      "intri-port-UpdateFlowControlConfig",
      AGT_RPC_PH_INVOKE,
      intri_port_Port_UpdateFlowControlConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_port_UpdateEnergyEfficiencyConfig_obj = ncx_find_object(
      intri_port_mod,
      "intri-port-UpdateEnergyEfficiencyConfig");
  if (intri_port_UpdateEnergyEfficiencyConfig_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_port,
      "intri-port-UpdateEnergyEfficiencyConfig",
      AGT_RPC_PH_INVOKE,
      intri_port_Port_UpdateEnergyEfficiencyConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_port_GetStatus_obj = ncx_find_object(
      intri_port_mod,
      "intri-port-GetStatus");
  if (intri_port_GetStatus_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_port,
      "intri-port-GetStatus",
      AGT_RPC_PH_INVOKE,
      intri_port_Port_GetStatus_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_port_RunRestartPorts_obj = ncx_find_object(
      intri_port_mod,
      "intri-port-RunRestartPorts");
  if (intri_port_RunRestartPorts_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_port,
      "intri-port-RunRestartPorts",
      AGT_RPC_PH_INVOKE,
      intri_port_Port_RunRestartPorts_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_port_GetLgStatus_obj = ncx_find_object(
      intri_port_mod,
      "intri-port-GetLgStatus");
  if (intri_port_GetLgStatus_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_port,
      "intri-port-GetLgStatus",
      AGT_RPC_PH_INVOKE,
      intri_port_Port_GetLgStatus_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_port_RunEnablePort_obj = ncx_find_object(
      intri_port_mod,
      "intri-port-RunEnablePort");
  if (intri_port_RunEnablePort_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_port,
      "intri-port-RunEnablePort",
      AGT_RPC_PH_INVOKE,
      intri_port_Port_RunEnablePort_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_port_GetPortStatus_obj = ncx_find_object(
      intri_port_mod,
      "intri-port-GetPortStatus");
  if (intri_port_GetPortStatus_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_port,
      "intri-port-GetPortStatus",
      AGT_RPC_PH_INVOKE,
      intri_port_Port_GetPortStatus_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_port_GetLgPortStatus_obj = ncx_find_object(
      intri_port_mod,
      "intri-port-GetLgPortStatus");
  if (intri_port_GetLgPortStatus_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_port,
      "intri-port-GetLgPortStatus",
      AGT_RPC_PH_INVOKE,
      intri_port_Port_GetLgPortStatus_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

void y_intri_port_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_port,
      "intri-port-GetConfig");

  agt_rpc_unregister_method(
      y_M_intri_port,
      "intri-port-UpdateAliasConfig");

  agt_rpc_unregister_method(
      y_M_intri_port,
      "intri-port-UpdateOperationConfig");

  agt_rpc_unregister_method(
      y_M_intri_port,
      "intri-port-UpdateSpeedDuplexConfig");

  agt_rpc_unregister_method(
      y_M_intri_port,
      "intri-port-UpdateFlowControlConfig");

  agt_rpc_unregister_method(
      y_M_intri_port,
      "intri-port-UpdateEnergyEfficiencyConfig");

  agt_rpc_unregister_method(
      y_M_intri_port,
      "intri-port-GetStatus");

  agt_rpc_unregister_method(
      y_M_intri_port,
      "intri-port-RunRestartPorts");

  agt_rpc_unregister_method(
      y_M_intri_port,
      "intri-port-GetLgStatus");

  agt_rpc_unregister_method(
      y_M_intri_port,
      "intri-port-RunEnablePort");

  agt_rpc_unregister_method(
      y_M_intri_port,
      "intri-port-GetPortStatus");

  agt_rpc_unregister_method(
      y_M_intri_port,
      "intri-port-GetLgPortStatus");

}
