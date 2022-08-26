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

#include "sercom-loop@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../../realize/github.com/Intrising/intri-type/device/intri-device@2022-06-24.h"
#include "../../../../../../realize/github.com/golang/protobuf/ptypes/empty/intri-empty@2022-06-24.h"
#include "../../../../../../realize/github.com/golang/protobuf/ptypes/timestamp/intri-timestamp@2022-06-24.h"

#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/loop/intri-loop-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/timestamp/intri-timestamp-trans.h"

static ncx_module_t *intri_loop_mod;

static status_t intri_loop_Loop_GetStatus_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct looppb_Status *out = malloc(sizeof(*out));

  loop_Loop_GetStatus(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_loop,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_loop_Status(outval, out);
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
static status_t intri_loop_Loop_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct looppb_Config *out = malloc(sizeof(*out));

  loop_Loop_GetConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_loop,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_loop_Config(outval, out);
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
static status_t intri_loop_Loop_GetPortConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct looppb_PortConfig *out = malloc(sizeof(*out));

  loop_Loop_GetPortConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_loop,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_loop_PortConfig(outval, out);
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
static status_t intri_loop_Loop_UpdatePortConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct looppb_PortConfigEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: the Get func has input */
  res = build_to_priv_loop_PortConfigEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  loop_Loop_UpdatePortConfig(in, out);

  free(in);
  free(out);
  return res;
}

status_t y_sercom_loop_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_loop)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }
  if (revision && xml_strcmp(revision, y_R_intri_loop)) {
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
    y_M_intri_timestamp,
    y_R_intri_timestamp,
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
      y_M_intri_loop,
      y_R_intri_loop,
      &agt_profile->agt_savedevQ,
      &intri_loop_mod);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_loop,
      "sercom-loop-Loop-GetStatus",
      AGT_RPC_PH_INVOKE,
      intri_loop_Loop_GetStatus_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_loop,
      "sercom-loop-Loop-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_loop_Loop_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_loop,
      "sercom-loop-Loop-GetPortConfig",
      AGT_RPC_PH_INVOKE,
      intri_loop_Loop_GetPortConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_loop,
      "sercom-loop-Loop-UpdatePortConfig",
      AGT_RPC_PH_INVOKE,
      intri_loop_Loop_UpdatePortConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

status_t y_sercom_loop_init2(void) {
  status_t res = NO_ERR;
  return res;
}

void y_sercom_loop_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_loop,
      "sercom-loop-Loop-GetStatus");
  agt_rpc_unregister_method(
      y_M_intri_loop,
      "sercom-loop-Loop-GetConfig");
  agt_rpc_unregister_method(
      y_M_intri_loop,
      "sercom-loop-Loop-GetPortConfig");
  agt_rpc_unregister_method(
      y_M_intri_loop,
      "sercom-loop-Loop-UpdatePortConfig");
}
