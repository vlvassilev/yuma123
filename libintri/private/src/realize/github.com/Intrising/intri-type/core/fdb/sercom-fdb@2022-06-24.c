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

#include "sercom-fdb@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../../realize/github.com/Intrising/intri-type/device/sercom-device@2022-06-24.h"
#include "../../../../../../realize/github.com/Intrising/intri-type/event/sercom-event@2022-06-24.h"
#include "../../../../../../realize/github.com/golang/protobuf/ptypes/empty/sercom-empty@2022-06-24.h"

#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/fdb/intri-fdb-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/event/intri-event-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_fdb_mod;

static status_t intri_fdb_FDB_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct fdbpb_Config *out = malloc(sizeof(*out));

  fdb_FDB_GetConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_fdb,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_fdb_Config(outval, out);
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
static status_t intri_fdb_FDB_GetMACAgingTime_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct fdbpb_AgingTime *out = malloc(sizeof(*out));

  fdb_FDB_GetMACAgingTime(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_fdb,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_fdb_AgingTime(outval, out);
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
static status_t intri_fdb_FDB_SetMACAgingTime_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_AgingTime *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  fdb_FDB_GetMACAgingTime(out, in);
  res = build_to_priv_fdb_AgingTime(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  fdb_FDB_SetMACAgingTime(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_fdb_FDB_GetFDBForwardTable_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct fdbpb_ForwardConfig *out = malloc(sizeof(*out));

  fdb_FDB_GetFDBForwardTable(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_fdb,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_fdb_ForwardConfig(outval, out);
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
static status_t intri_fdb_FDB_AddForwardMACAddress_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_ForwardConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_fdb_ForwardConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  fdb_FDB_AddForwardMACAddress(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_fdb_FDB_DeleteForwardMACAddress_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_ForwardConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_fdb_ForwardConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  fdb_FDB_DeleteForwardMACAddress(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_fdb_FDB_UpdateForwardMACAddress_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_ForwardConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_fdb_ForwardConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  fdb_FDB_UpdateForwardMACAddress(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_fdb_FDB_GetFDBDropTable_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct fdbpb_DropConfig *out = malloc(sizeof(*out));

  fdb_FDB_GetFDBDropTable(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_fdb,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_fdb_DropConfig(outval, out);
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
static status_t intri_fdb_FDB_AddDropMACAddress_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_DropConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_fdb_DropConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  fdb_FDB_AddDropMACAddress(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_fdb_FDB_DeleteDropMACAddress_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_DropConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_fdb_DropConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  fdb_FDB_DeleteDropMACAddress(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_fdb_FDB_UpdateDropMACAddress_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_DropConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_fdb_DropConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  fdb_FDB_UpdateDropMACAddress(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_fdb_FDB_UpdatePortLearningLimit_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_PortLearningLimit *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_fdb_PortLearningLimit(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  fdb_FDB_UpdatePortLearningLimit(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_fdb_FDB_GetInfo_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct fdbpb_Info *out = malloc(sizeof(*out));

  fdb_FDB_GetInfo(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_fdb,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_fdb_Info(outval, out);
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
static status_t intri_fdb_FDB_GetTable_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct fdbpb_Status *out = malloc(sizeof(*out));

  fdb_FDB_GetTable(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_fdb,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_fdb_Status(outval, out);
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
static status_t intri_fdb_FDB_GetAuthorizedTable_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct fdbpb_Status *out = malloc(sizeof(*out));

  fdb_FDB_GetAuthorizedTable(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_fdb,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_fdb_Status(outval, out);
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
static status_t intri_fdb_FDB_GetSecurityTable_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct fdbpb_Status *out = malloc(sizeof(*out));

  fdb_FDB_GetSecurityTable(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_fdb,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_fdb_Status(outval, out);
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
static status_t intri_fdb_FDB_GetSpecificTable_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_SpecificMac *in = malloc(sizeof(*in));
  struct fdbpb_Status *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_fdb_SpecificMac(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  fdb_FDB_GetSpecificTable(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_fdb,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_fdb_Status(outval, out);
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
static status_t intri_fdb_FDB_RunClearMACTable_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  fdb_FDB_RunClearMACTable(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_fdb_FDB_RunClearMACTableForInterface_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_FlushOption *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_fdb_FlushOption(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  fdb_FDB_RunClearMACTableForInterface(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_fdb_FDB_AddEntry_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct eventpb_FDBEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_event_FDBEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  fdb_FDB_AddEntry(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_fdb_FDB_DeleteEntry_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct eventpb_FDBEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_event_FDBEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  fdb_FDB_DeleteEntry(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_fdb_FDB_UpdateEntry_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct eventpb_FDBEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_event_FDBEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  fdb_FDB_UpdateEntry(in, out);

  free(in);
  free(out);
  return res;
}

status_t y_sercom_fdb_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_fdb)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }
  if (revision && xml_strcmp(revision, y_R_intri_fdb)) {
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

  res = agt_load_sil_code(
    y_M_intri_event,
    y_R_intri_event,
    true);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = ncxmod_load_module(
      y_M_intri_fdb,
      y_R_intri_fdb,
      &agt_profile->agt_savedevQ,
      &intri_fdb_mod);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetMACAgingTime",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetMACAgingTime_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-SetMACAgingTime",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_SetMACAgingTime_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetFDBForwardTable",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetFDBForwardTable_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-AddForwardMACAddress",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_AddForwardMACAddress_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-DeleteForwardMACAddress",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_DeleteForwardMACAddress_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-UpdateForwardMACAddress",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_UpdateForwardMACAddress_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetFDBDropTable",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetFDBDropTable_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-AddDropMACAddress",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_AddDropMACAddress_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-DeleteDropMACAddress",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_DeleteDropMACAddress_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-UpdateDropMACAddress",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_UpdateDropMACAddress_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-UpdatePortLearningLimit",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_UpdatePortLearningLimit_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetInfo",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetInfo_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetTable",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetTable_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetAuthorizedTable",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetAuthorizedTable_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetSecurityTable",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetSecurityTable_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetSpecificTable",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetSpecificTable_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-RunClearMACTable",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_RunClearMACTable_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-RunClearMACTableForInterface",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_RunClearMACTableForInterface_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-AddEntry",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_AddEntry_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-DeleteEntry",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_DeleteEntry_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-UpdateEntry",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_UpdateEntry_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

status_t y_sercom_fdb_init2(void) {
  status_t res = NO_ERR;
  return res;
}

void y_sercom_fdb_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetConfig");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetMACAgingTime");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-SetMACAgingTime");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetFDBForwardTable");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-AddForwardMACAddress");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-DeleteForwardMACAddress");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-UpdateForwardMACAddress");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetFDBDropTable");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-AddDropMACAddress");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-DeleteDropMACAddress");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-UpdateDropMACAddress");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-UpdatePortLearningLimit");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetInfo");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetTable");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetAuthorizedTable");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetSecurityTable");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-GetSpecificTable");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-RunClearMACTable");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-RunClearMACTableForInterface");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-AddEntry");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-DeleteEntry");
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "sercom-fdb-FDB-UpdateEntry");
}
