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

#include "intri-mirroring@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../../genc-trans/github.com/Intrising/intri-type/common/intri-common-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/mirroring/intri-mirroring-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_mirroring_mod;

static status_t intri_mirroring_Mirroring_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct mirroringpb_Config *out = malloc(sizeof(*out));

  mirroring_Mirroring_GetConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_mirroring,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_mirroring_Config(outval, out);
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
static status_t intri_mirroring_Mirroring_GetRSPAN_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct mirroringpb_RSPANConfig *out = malloc(sizeof(*out));

  mirroring_Mirroring_GetRSPAN(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_mirroring,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_mirroring_RSPANConfig(outval, out);
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
static status_t intri_mirroring_Mirroring_SetRSPAN_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct mirroringpb_RSPANConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  mirroring_Mirroring_GetRSPAN(out, in);
  res = build_to_priv_mirroring_RSPANConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  mirroring_Mirroring_SetRSPAN(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_mirroring_Mirroring_GetDestinationList_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct mirroringpb_DestinationSession *out = malloc(sizeof(*out));

  mirroring_Mirroring_GetDestinationList(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_mirroring,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_mirroring_DestinationSession(outval, out);
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
static status_t intri_mirroring_Mirroring_AddDestination_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct mirroringpb_DestinationSessionEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_mirroring_DestinationSessionEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  mirroring_Mirroring_AddDestination(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_mirroring_Mirroring_DeleteDestination_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct commonpb_Index *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_common_Index(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  mirroring_Mirroring_DeleteDestination(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_mirroring_Mirroring_UpdateDestination_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct mirroringpb_DestinationSessionEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_mirroring_DestinationSessionEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  mirroring_Mirroring_UpdateDestination(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_mirroring_Mirroring_GetSourceList_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct mirroringpb_SourceSession *out = malloc(sizeof(*out));

  mirroring_Mirroring_GetSourceList(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_mirroring,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_mirroring_SourceSession(outval, out);
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
static status_t intri_mirroring_Mirroring_AddSource_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct mirroringpb_SourceSessionEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_mirroring_SourceSessionEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  mirroring_Mirroring_AddSource(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_mirroring_Mirroring_DeleteSource_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct mirroringpb_SourceSessionEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_mirroring_SourceSessionEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  mirroring_Mirroring_DeleteSource(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_mirroring_Mirroring_UpdateSource_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct mirroringpb_SourceSessionEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_mirroring_SourceSessionEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  mirroring_Mirroring_UpdateSource(in, out);

  free(in);
  free(out);
  return res;
}

status_t y_intri_mirroring_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_mirroring)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }
  if (revision && xml_strcmp(revision, y_R_intri_mirroring)) {
    return ERR_NCX_WRONG_VERSION;
  }

  res = ncxmod_load_module(
      y_M_intri_mirroring,
      y_R_intri_mirroring,
      &agt_profile->agt_savedevQ,
      &intri_mirroring_mod);
  if (res != NO_ERR) {
    return res;
  }

  res = agt_rpc_register_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_mirroring_Mirroring_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-GetRSPAN",
      AGT_RPC_PH_INVOKE,
      intri_mirroring_Mirroring_GetRSPAN_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-SetRSPAN",
      AGT_RPC_PH_INVOKE,
      intri_mirroring_Mirroring_SetRSPAN_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-GetDestinationList",
      AGT_RPC_PH_INVOKE,
      intri_mirroring_Mirroring_GetDestinationList_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-AddDestination",
      AGT_RPC_PH_INVOKE,
      intri_mirroring_Mirroring_AddDestination_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-DeleteDestination",
      AGT_RPC_PH_INVOKE,
      intri_mirroring_Mirroring_DeleteDestination_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-UpdateDestination",
      AGT_RPC_PH_INVOKE,
      intri_mirroring_Mirroring_UpdateDestination_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-GetSourceList",
      AGT_RPC_PH_INVOKE,
      intri_mirroring_Mirroring_GetSourceList_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-AddSource",
      AGT_RPC_PH_INVOKE,
      intri_mirroring_Mirroring_AddSource_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-DeleteSource",
      AGT_RPC_PH_INVOKE,
      intri_mirroring_Mirroring_DeleteSource_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-UpdateSource",
      AGT_RPC_PH_INVOKE,
      intri_mirroring_Mirroring_UpdateSource_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

status_t y_intri_mirroring_init2(void) {
  status_t res = NO_ERR;
  return res;
}

void y_intri_mirroring_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-GetConfig");
  agt_rpc_unregister_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-GetRSPAN");
  agt_rpc_unregister_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-SetRSPAN");
  agt_rpc_unregister_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-GetDestinationList");
  agt_rpc_unregister_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-AddDestination");
  agt_rpc_unregister_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-DeleteDestination");
  agt_rpc_unregister_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-UpdateDestination");
  agt_rpc_unregister_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-GetSourceList");
  agt_rpc_unregister_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-AddSource");
  agt_rpc_unregister_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-DeleteSource");
  agt_rpc_unregister_method(
      y_M_intri_mirroring,
      "intri-mirroring-Mirroring-UpdateSource");
}
