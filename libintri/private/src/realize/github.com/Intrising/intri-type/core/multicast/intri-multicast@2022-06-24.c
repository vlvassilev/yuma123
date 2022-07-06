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

#include "intri-multicast@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../../genc-trans/github.com/Intrising/intri-type/common/intri-common-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/multicast/intri-multicast-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_multicast_mod;

static status_t intri_multicast_Multicast_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct multicastpb_Config *out = malloc(sizeof(*out));

  multicast_Multicast_GetConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_multicast,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_multicast_Config(outval, out);
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
static status_t intri_multicast_Multicast_SetIGMPSnoopingEnabled_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct commonpb_Enabled *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_common_Enabled(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_SetIGMPSnoopingEnabled(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_multicast_Multicast_SetMLDSnoopingEnabled_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct commonpb_Enabled *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_common_Enabled(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_SetMLDSnoopingEnabled(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_multicast_Multicast_GetIGMPSnoopingConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_VlanList *in = malloc(sizeof(*in));
  struct multicastpb_Snooping *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_multicast_VlanList(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_GetIGMPSnoopingConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_multicast,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_multicast_Snooping(outval, out);
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
static status_t intri_multicast_Multicast_SetIGMPSnoopingConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_Snooping *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: the Get func has input */
  res = build_to_priv_multicast_Snooping(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_SetIGMPSnoopingConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_multicast_Multicast_GetMLDSnoopingConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_VlanList *in = malloc(sizeof(*in));
  struct multicastpb_Snooping *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_multicast_VlanList(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_GetMLDSnoopingConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_multicast,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_multicast_Snooping(outval, out);
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
static status_t intri_multicast_Multicast_SetMLDSnoopingConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_Snooping *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: the Get func has input */
  res = build_to_priv_multicast_Snooping(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_SetMLDSnoopingConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_multicast_Multicast_GetStaticGroupConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_VlanList *in = malloc(sizeof(*in));
  struct multicastpb_Static *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_multicast_VlanList(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_GetStaticGroupConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_multicast,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_multicast_Static(outval, out);
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
static status_t intri_multicast_Multicast_UpdateStaticGroupConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_Static *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: the Get func has input */
  res = build_to_priv_multicast_Static(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_UpdateStaticGroupConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_multicast_Multicast_AddStaticGroupConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_Static *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_multicast_Static(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_AddStaticGroupConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_multicast_Multicast_DeleteStaticGroupConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_Static *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_multicast_Static(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_DeleteStaticGroupConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_multicast_Multicast_GetUnregisterFloodingConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_VlanList *in = malloc(sizeof(*in));
  struct multicastpb_UnregisterFlood *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_multicast_VlanList(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_GetUnregisterFloodingConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_multicast,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_multicast_UnregisterFlood(outval, out);
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
static status_t intri_multicast_Multicast_SetUnregisterFloodingConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_UnregisterFlood *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: the Get func has input */
  res = build_to_priv_multicast_UnregisterFlood(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_SetUnregisterFloodingConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_multicast_Multicast_GetRouterPortStatus_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_VlanList *in = malloc(sizeof(*in));
  struct multicastpb_RouterStatus *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_multicast_VlanList(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_GetRouterPortStatus(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_multicast,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_multicast_RouterStatus(outval, out);
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
static status_t intri_multicast_Multicast_SetRouterPortConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_RouterPort *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: the Get func has input */
  res = build_to_priv_multicast_RouterPort(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_SetRouterPortConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_multicast_Multicast_GetRouterPortConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_VlanList *in = malloc(sizeof(*in));
  struct multicastpb_RouterPort *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_multicast_VlanList(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_GetRouterPortConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_multicast,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_multicast_RouterPort(outval, out);
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
static status_t intri_multicast_Multicast_GetIGMPGroups_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_VlanList *in = malloc(sizeof(*in));
  struct multicastpb_DynamicGroups *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_multicast_VlanList(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_GetIGMPGroups(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_multicast,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_multicast_DynamicGroups(outval, out);
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
static status_t intri_multicast_Multicast_GetMLDGroups_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_VlanList *in = malloc(sizeof(*in));
  struct multicastpb_DynamicGroups *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_multicast_VlanList(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_GetMLDGroups(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_multicast,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_multicast_DynamicGroups(outval, out);
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
static status_t intri_multicast_Multicast_GetIGMPStatistics_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_VlanList *in = malloc(sizeof(*in));
  struct multicastpb_IGMPStatistics *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_multicast_VlanList(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_GetIGMPStatistics(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_multicast,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_multicast_IGMPStatistics(outval, out);
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
static status_t intri_multicast_Multicast_GetMLDStatistics_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct multicastpb_VlanList *in = malloc(sizeof(*in));
  struct multicastpb_MLDStatistics *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_multicast_VlanList(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  multicast_Multicast_GetMLDStatistics(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_multicast,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_multicast_MLDStatistics(outval, out);
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

status_t y_intri_multicast_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_multicast)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }
  if (revision && xml_strcmp(revision, y_R_intri_multicast)) {
    return ERR_NCX_WRONG_VERSION;
  }

  res = ncxmod_load_module(
      y_M_intri_multicast,
      y_R_intri_multicast,
      &agt_profile->agt_savedevQ,
      &intri_multicast_mod);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-SetIGMPSnoopingEnabled",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_SetIGMPSnoopingEnabled_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-SetMLDSnoopingEnabled",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_SetMLDSnoopingEnabled_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetIGMPSnoopingConfig",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_GetIGMPSnoopingConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-SetIGMPSnoopingConfig",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_SetIGMPSnoopingConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetMLDSnoopingConfig",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_GetMLDSnoopingConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-SetMLDSnoopingConfig",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_SetMLDSnoopingConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetStaticGroupConfig",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_GetStaticGroupConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-UpdateStaticGroupConfig",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_UpdateStaticGroupConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-AddStaticGroupConfig",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_AddStaticGroupConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-DeleteStaticGroupConfig",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_DeleteStaticGroupConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetUnregisterFloodingConfig",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_GetUnregisterFloodingConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-SetUnregisterFloodingConfig",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_SetUnregisterFloodingConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetRouterPortStatus",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_GetRouterPortStatus_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-SetRouterPortConfig",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_SetRouterPortConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetRouterPortConfig",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_GetRouterPortConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetIGMPGroups",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_GetIGMPGroups_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetMLDGroups",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_GetMLDGroups_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetIGMPStatistics",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_GetIGMPStatistics_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetMLDStatistics",
      AGT_RPC_PH_INVOKE,
      intri_multicast_Multicast_GetMLDStatistics_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

status_t y_intri_multicast_init2(void) {
  status_t res = NO_ERR;
  return res;
}

void y_intri_multicast_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetConfig");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-SetIGMPSnoopingEnabled");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-SetMLDSnoopingEnabled");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetIGMPSnoopingConfig");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-SetIGMPSnoopingConfig");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetMLDSnoopingConfig");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-SetMLDSnoopingConfig");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetStaticGroupConfig");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-UpdateStaticGroupConfig");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-AddStaticGroupConfig");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-DeleteStaticGroupConfig");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetUnregisterFloodingConfig");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-SetUnregisterFloodingConfig");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetRouterPortStatus");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-SetRouterPortConfig");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetRouterPortConfig");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetIGMPGroups");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetMLDGroups");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetIGMPStatistics");
  agt_rpc_unregister_method(
      y_M_intri_multicast,
      "intri-multicast-Multicast-GetMLDStatistics");
}
