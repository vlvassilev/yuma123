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

#include "sercom-dhcp@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../../realize/github.com/Intrising/intri-type/device/intri-device@2022-06-24.h"
#include "../../../../../../realize/github.com/golang/protobuf/ptypes/empty/intri-empty@2022-06-24.h"

#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/dhcp/intri-dhcp-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_dhcp_mod;

static status_t intri_dhcp_ARPInspection_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct dhcppb_ARPInspectionConfig *out = malloc(sizeof(*out));

  dhcp_ARPInspection_GetConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_dhcp,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_dhcp_ARPInspectionConfig(outval, out);
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
static status_t intri_dhcp_ARPInspection_SetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct dhcppb_ARPInspectionConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  dhcp_ARPInspection_GetConfig(out, in);
  res = build_to_priv_dhcp_ARPInspectionConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  dhcp_ARPInspection_SetConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_dhcp_DHCP_GetRelayConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct dhcppb_RelayConfig *out = malloc(sizeof(*out));

  dhcp_DHCP_GetRelayConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_dhcp,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_dhcp_RelayConfig(outval, out);
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
static status_t intri_dhcp_DHCP_GetSnoopingConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct dhcppb_SnoopingConfig *out = malloc(sizeof(*out));

  dhcp_DHCP_GetSnoopingConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_dhcp,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_dhcp_SnoopingConfig(outval, out);
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
static status_t intri_dhcp_DHCP_SetRelayConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct dhcppb_RelayConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  dhcp_DHCP_GetRelayConfig(out, in);
  res = build_to_priv_dhcp_RelayConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  dhcp_DHCP_SetRelayConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_dhcp_DHCP_SetSnoopingConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct dhcppb_SnoopingConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  dhcp_DHCP_GetSnoopingConfig(out, in);
  res = build_to_priv_dhcp_SnoopingConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  dhcp_DHCP_SetSnoopingConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_dhcp_DHCP_RunUnblockPort_invoke(
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
  dhcp_DHCP_RunUnblockPort(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_dhcp_DHCP_GetSnoopingStatistic_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct dhcppb_SnoopingStatisticsList *out = malloc(sizeof(*out));

  dhcp_DHCP_GetSnoopingStatistic(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_dhcp,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_dhcp_SnoopingStatisticsList(outval, out);
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
static status_t intri_dhcp_DHCP_GetSnoopingBindingDatabase_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct dhcppb_SnoopingBindingDatabaseList *out = malloc(sizeof(*out));

  dhcp_DHCP_GetSnoopingBindingDatabase(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_dhcp,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_dhcp_SnoopingBindingDatabaseList(outval, out);
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
static status_t intri_dhcp_DHCP_ClearSnoopingStatistic_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  dhcp_DHCP_ClearSnoopingStatistic(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_dhcp_DHCP_ClearSnoopingBindingDatabase_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  dhcp_DHCP_ClearSnoopingBindingDatabase(in, out);

  free(in);
  free(out);
  return res;
}

status_t y_sercom_dhcp_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_dhcp)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }
  if (revision && xml_strcmp(revision, y_R_intri_dhcp)) {
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
      y_M_intri_dhcp,
      y_R_intri_dhcp,
      &agt_profile->agt_savedevQ,
      &intri_dhcp_mod);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_dhcp,
      "sercom-dhcp-ARPInspection-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_dhcp_ARPInspection_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_dhcp,
      "sercom-dhcp-ARPInspection-SetConfig",
      AGT_RPC_PH_INVOKE,
      intri_dhcp_ARPInspection_SetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-GetRelayConfig",
      AGT_RPC_PH_INVOKE,
      intri_dhcp_DHCP_GetRelayConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-GetSnoopingConfig",
      AGT_RPC_PH_INVOKE,
      intri_dhcp_DHCP_GetSnoopingConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-SetRelayConfig",
      AGT_RPC_PH_INVOKE,
      intri_dhcp_DHCP_SetRelayConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-SetSnoopingConfig",
      AGT_RPC_PH_INVOKE,
      intri_dhcp_DHCP_SetSnoopingConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-RunUnblockPort",
      AGT_RPC_PH_INVOKE,
      intri_dhcp_DHCP_RunUnblockPort_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-GetSnoopingStatistic",
      AGT_RPC_PH_INVOKE,
      intri_dhcp_DHCP_GetSnoopingStatistic_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-GetSnoopingBindingDatabase",
      AGT_RPC_PH_INVOKE,
      intri_dhcp_DHCP_GetSnoopingBindingDatabase_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-ClearSnoopingStatistic",
      AGT_RPC_PH_INVOKE,
      intri_dhcp_DHCP_ClearSnoopingStatistic_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-ClearSnoopingBindingDatabase",
      AGT_RPC_PH_INVOKE,
      intri_dhcp_DHCP_ClearSnoopingBindingDatabase_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

status_t y_sercom_dhcp_init2(void) {
  status_t res = NO_ERR;
  return res;
}

void y_sercom_dhcp_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_dhcp,
      "sercom-dhcp-ARPInspection-GetConfig");
  agt_rpc_unregister_method(
      y_M_intri_dhcp,
      "sercom-dhcp-ARPInspection-SetConfig");
  agt_rpc_unregister_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-GetRelayConfig");
  agt_rpc_unregister_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-GetSnoopingConfig");
  agt_rpc_unregister_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-SetRelayConfig");
  agt_rpc_unregister_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-SetSnoopingConfig");
  agt_rpc_unregister_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-RunUnblockPort");
  agt_rpc_unregister_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-GetSnoopingStatistic");
  agt_rpc_unregister_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-GetSnoopingBindingDatabase");
  agt_rpc_unregister_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-ClearSnoopingStatistic");
  agt_rpc_unregister_method(
      y_M_intri_dhcp,
      "sercom-dhcp-DHCP-ClearSnoopingBindingDatabase");
}
