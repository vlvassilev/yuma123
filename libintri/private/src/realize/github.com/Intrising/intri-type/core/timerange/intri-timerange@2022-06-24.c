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

#include "intri-timerange@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../../genc-trans/github.com/Intrising/intri-type/common/intri-common-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/timerange/intri-timerange-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"
#include "../../../../../../genc-trans/google.golang.org/genproto/googleapis/type/date/intri-date-trans.h"
#include "../../../../../../genc-trans/google.golang.org/genproto/googleapis/type/dayofweek/intri-dayofweek-trans.h"
#include "../../../../../../genc-trans/google.golang.org/genproto/googleapis/type/timeofday/intri-timeofday-trans.h"

static ncx_module_t *intri_timerange_mod;

static status_t intri_timerange_TimeRange_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct timerangepb_Config *out = malloc(sizeof(*out));

  timerange_TimeRange_GetConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_timerange,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_timerange_Config(outval, out);
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
static status_t intri_timerange_TimeRange_GetStatus_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct timerangepb_Status *out = malloc(sizeof(*out));

  timerange_TimeRange_GetStatus(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_timerange,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_timerange_Status(outval, out);
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
static status_t intri_timerange_TimeRange_AddTimeRange_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct timerangepb_ConfigEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_timerange_ConfigEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  timerange_TimeRange_AddTimeRange(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_timerange_TimeRange_DeleteTimeRange_invoke(
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
  timerange_TimeRange_DeleteTimeRange(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_timerange_TimeRange_UpdateTimeRange_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct timerangepb_ConfigEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_timerange_ConfigEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  timerange_TimeRange_UpdateTimeRange(in, out);

  free(in);
  free(out);
  return res;
}

status_t y_intri_timerange_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_timerange)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }
  if (revision && xml_strcmp(revision, y_R_intri_timerange)) {
    return ERR_NCX_WRONG_VERSION;
  }

  res = ncxmod_load_module(
      y_M_intri_timerange,
      y_R_intri_timerange,
      &agt_profile->agt_savedevQ,
      &intri_timerange_mod);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_timerange,
      "intri-timerange-TimeRange-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_timerange_TimeRange_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_timerange,
      "intri-timerange-TimeRange-GetStatus",
      AGT_RPC_PH_INVOKE,
      intri_timerange_TimeRange_GetStatus_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_timerange,
      "intri-timerange-TimeRange-AddTimeRange",
      AGT_RPC_PH_INVOKE,
      intri_timerange_TimeRange_AddTimeRange_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_timerange,
      "intri-timerange-TimeRange-DeleteTimeRange",
      AGT_RPC_PH_INVOKE,
      intri_timerange_TimeRange_DeleteTimeRange_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_timerange,
      "intri-timerange-TimeRange-UpdateTimeRange",
      AGT_RPC_PH_INVOKE,
      intri_timerange_TimeRange_UpdateTimeRange_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

status_t y_intri_timerange_init2(void) {
  status_t res = NO_ERR;
  return res;
}

void y_intri_timerange_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_timerange,
      "intri-timerange-TimeRange-GetConfig");
  agt_rpc_unregister_method(
      y_M_intri_timerange,
      "intri-timerange-TimeRange-GetStatus");
  agt_rpc_unregister_method(
      y_M_intri_timerange,
      "intri-timerange-TimeRange-AddTimeRange");
  agt_rpc_unregister_method(
      y_M_intri_timerange,
      "intri-timerange-TimeRange-DeleteTimeRange");
  agt_rpc_unregister_method(
      y_M_intri_timerange,
      "intri-timerange-TimeRange-UpdateTimeRange");
}
