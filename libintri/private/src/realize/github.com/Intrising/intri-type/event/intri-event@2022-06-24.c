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

#include "intri-event@2022-06-24.h"
#include "../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../genc-trans/github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../genc-trans/github.com/Intrising/intri-type/event/intri-event-trans.h"
#include "../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"
#include "../../../../../genc-trans/github.com/golang/protobuf/ptypes/timestamp/intri-timestamp-trans.h"

static ncx_module_t *intri_event_mod;

static status_t intri_event_Event_WaitInLine_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct eventpb_ServiceInitialized *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_event_ServiceInitialized(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  event_Event_WaitInLine(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_event_Event_SetManagmentVLANPriority_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct eventpb_ManagmentVLANPriority *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_event_ManagmentVLANPriority(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  event_Event_SetManagmentVLANPriority(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_event_Event_EncodeDecode_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct eventpb_CryptoRequest *in = malloc(sizeof(*in));
  struct eventpb_CryptoResponse *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_event_CryptoRequest(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  event_Event_EncodeDecode(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_event,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_event_CryptoResponse(outval, out);
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
static status_t intri_event_Event_Base64Encode_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct eventpb_CryptoBase64Request *in = malloc(sizeof(*in));
  struct eventpb_CryptoBase64Response *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_event_CryptoBase64Request(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  event_Event_Base64Encode(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_event,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_event_CryptoBase64Response(outval, out);
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
static status_t intri_event_Event_Base64Decode_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct eventpb_CryptoBase64Request *in = malloc(sizeof(*in));
  struct eventpb_CryptoBase64Response *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_event_CryptoBase64Request(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  event_Event_Base64Decode(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_event,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_event_CryptoBase64Response(outval, out);
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

status_t y_intri_event_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_event)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }
  if (revision && xml_strcmp(revision, y_R_intri_event)) {
    return ERR_NCX_WRONG_VERSION;
  }

  res = ncxmod_load_module(
      y_M_intri_event,
      y_R_intri_event,
      &agt_profile->agt_savedevQ,
      &intri_event_mod);
  if (res != NO_ERR) {
    return res;
  }

  res = agt_rpc_register_method(
      y_M_intri_event,
      "intri-event-Event-WaitInLine",
      AGT_RPC_PH_INVOKE,
      intri_event_Event_WaitInLine_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_event,
      "intri-event-Event-SetManagmentVLANPriority",
      AGT_RPC_PH_INVOKE,
      intri_event_Event_SetManagmentVLANPriority_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_event,
      "intri-event-Event-EncodeDecode",
      AGT_RPC_PH_INVOKE,
      intri_event_Event_EncodeDecode_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_event,
      "intri-event-Event-Base64Encode",
      AGT_RPC_PH_INVOKE,
      intri_event_Event_Base64Encode_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_event,
      "intri-event-Event-Base64Decode",
      AGT_RPC_PH_INVOKE,
      intri_event_Event_Base64Decode_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

status_t y_intri_event_init2(void) {
  status_t res = NO_ERR;
  return res;
}

void y_intri_event_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_event,
      "intri-event-Event-WaitInLine");
  agt_rpc_unregister_method(
      y_M_intri_event,
      "intri-event-Event-SetManagmentVLANPriority");
  agt_rpc_unregister_method(
      y_M_intri_event,
      "intri-event-Event-EncodeDecode");
  agt_rpc_unregister_method(
      y_M_intri_event,
      "intri-event-Event-Base64Encode");
  agt_rpc_unregister_method(
      y_M_intri_event,
      "intri-event-Event-Base64Decode");
}
