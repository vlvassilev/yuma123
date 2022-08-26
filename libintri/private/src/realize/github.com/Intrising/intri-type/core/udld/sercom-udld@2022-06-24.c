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

#include "sercom-udld@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../../realize/github.com/Intrising/intri-type/device/intri-device@2022-06-24.h"
#include "../../../../../../realize/github.com/golang/protobuf/ptypes/empty/intri-empty@2022-06-24.h"

#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/udld/intri-udld-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_udld_mod;

static status_t intri_udld_UDLD_GetStatus_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct udldpb_Status *out = malloc(sizeof(*out));

  udld_UDLD_GetStatus(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_udld,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_udld_Status(outval, out);
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
static status_t intri_udld_UDLD_RunRestorePortStatus_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct devicepb_PortList *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_device_PortList(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  udld_UDLD_RunRestorePortStatus(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_udld_UDLD_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct udldpb_Config *out = malloc(sizeof(*out));

  udld_UDLD_GetConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_udld,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_udld_Config(outval, out);
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
static status_t intri_udld_UDLD_GetBasicConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct udldpb_BasicConfig *out = malloc(sizeof(*out));

  udld_UDLD_GetBasicConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_udld,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_udld_BasicConfig(outval, out);
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
static status_t intri_udld_UDLD_UpdateBasicConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct udldpb_BasicConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  udld_UDLD_GetBasicConfig(out, in);
  res = build_to_priv_udld_BasicConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  udld_UDLD_UpdateBasicConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_udld_UDLD_GetPortConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct udldpb_PortConfig *out = malloc(sizeof(*out));

  udld_UDLD_GetPortConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_udld,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_udld_PortConfig(outval, out);
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
static status_t intri_udld_UDLD_UpdatePortConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct udldpb_PortConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  udld_UDLD_GetPortConfig(out, in);
  res = build_to_priv_udld_PortConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  udld_UDLD_UpdatePortConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_udld_UDLD_UpdatePortConfigEntry_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct udldpb_PortConfigEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_udld_PortConfigEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  udld_UDLD_UpdatePortConfigEntry(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_udld_UDLD_GetStatistics_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct udldpb_Statistics *out = malloc(sizeof(*out));

  udld_UDLD_GetStatistics(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_udld,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_udld_Statistics(outval, out);
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
static status_t intri_udld_UDLD_RunClearStatistics_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct devicepb_PortList *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_device_PortList(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  udld_UDLD_RunClearStatistics(in, out);

  free(in);
  free(out);
  return res;
}

status_t y_sercom_udld_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_udld)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }
  if (revision && xml_strcmp(revision, y_R_intri_udld)) {
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
      y_M_intri_udld,
      y_R_intri_udld,
      &agt_profile->agt_savedevQ,
      &intri_udld_mod);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-GetStatus",
      AGT_RPC_PH_INVOKE,
      intri_udld_UDLD_GetStatus_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-RunRestorePortStatus",
      AGT_RPC_PH_INVOKE,
      intri_udld_UDLD_RunRestorePortStatus_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_udld_UDLD_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-GetBasicConfig",
      AGT_RPC_PH_INVOKE,
      intri_udld_UDLD_GetBasicConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-UpdateBasicConfig",
      AGT_RPC_PH_INVOKE,
      intri_udld_UDLD_UpdateBasicConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-GetPortConfig",
      AGT_RPC_PH_INVOKE,
      intri_udld_UDLD_GetPortConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-UpdatePortConfig",
      AGT_RPC_PH_INVOKE,
      intri_udld_UDLD_UpdatePortConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-UpdatePortConfigEntry",
      AGT_RPC_PH_INVOKE,
      intri_udld_UDLD_UpdatePortConfigEntry_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-GetStatistics",
      AGT_RPC_PH_INVOKE,
      intri_udld_UDLD_GetStatistics_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-RunClearStatistics",
      AGT_RPC_PH_INVOKE,
      intri_udld_UDLD_RunClearStatistics_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

status_t y_sercom_udld_init2(void) {
  status_t res = NO_ERR;
  return res;
}

void y_sercom_udld_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-GetStatus");
  agt_rpc_unregister_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-RunRestorePortStatus");
  agt_rpc_unregister_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-GetConfig");
  agt_rpc_unregister_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-GetBasicConfig");
  agt_rpc_unregister_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-UpdateBasicConfig");
  agt_rpc_unregister_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-GetPortConfig");
  agt_rpc_unregister_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-UpdatePortConfig");
  agt_rpc_unregister_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-UpdatePortConfigEntry");
  agt_rpc_unregister_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-GetStatistics");
  agt_rpc_unregister_method(
      y_M_intri_udld,
      "sercom-udld-UDLD-RunClearStatistics");
}
