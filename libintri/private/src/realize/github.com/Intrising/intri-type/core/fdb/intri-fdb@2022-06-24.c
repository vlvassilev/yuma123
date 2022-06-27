
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

#include "intri-fdb@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/fdb/intri-fdb-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/event/intri-event-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_fdb_mod;
static obj_template_t *intri_fdb_GetConfig_obj;
static obj_template_t *intri_fdb_GetMACAgingTime_obj;
static obj_template_t *intri_fdb_SetMACAgingTime_obj;
static obj_template_t *intri_fdb_GetFDBForwardTable_obj;
static obj_template_t *intri_fdb_AddForwardMACAddress_obj;
static obj_template_t *intri_fdb_DeleteForwardMACAddress_obj;
static obj_template_t *intri_fdb_UpdateForwardMACAddress_obj;
static obj_template_t *intri_fdb_GetFDBDropTable_obj;
static obj_template_t *intri_fdb_AddDropMACAddress_obj;
static obj_template_t *intri_fdb_DeleteDropMACAddress_obj;
static obj_template_t *intri_fdb_UpdateDropMACAddress_obj;
static obj_template_t *intri_fdb_UpdatePortLearningLimit_obj;
static obj_template_t *intri_fdb_GetInfo_obj;
static obj_template_t *intri_fdb_GetTable_obj;
static obj_template_t *intri_fdb_GetAuthorizedTable_obj;
static obj_template_t *intri_fdb_GetSecurityTable_obj;
static obj_template_t *intri_fdb_GetSpecificTable_obj;
static obj_template_t *intri_fdb_RunClearMACTable_obj;
static obj_template_t *intri_fdb_RunClearMACTableForInterface_obj;
static obj_template_t *intri_fdb_AddEntry_obj;
static obj_template_t *intri_fdb_DeleteEntry_obj;
static obj_template_t *intri_fdb_UpdateEntry_obj;
static obj_template_t *intri_fdb_UpdatePortOccupied_obj;
static obj_template_t *intri_fdb_AddMulticastEntry_obj;
static obj_template_t *intri_fdb_DeleteMulticastEntry_obj;
static obj_template_t *intri_fdb_UpdateMulticastEntry_obj;

static status_t intri_fdb_FDB_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct fdbpb_Config out;
  fdb_FDB_GetConfig(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_fdb_Config(
      resval,
    &out);
  return res;
}

static status_t intri_fdb_FDB_GetMACAgingTime_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct fdbpb_AgingTime out;
  fdb_FDB_GetMACAgingTime(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_fdb_AgingTime(
      resval,
    &out);
  return res;
}

static status_t intri_fdb_FDB_SetMACAgingTime_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_AgingTime in;
  struct emptypb_Empty out;
  fdb_FDB_SetMACAgingTime(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_GetFDBForwardTable_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct fdbpb_ForwardConfig out;
  fdb_FDB_GetFDBForwardTable(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_fdb_ForwardConfig(
      resval,
    &out);
  return res;
}

static status_t intri_fdb_FDB_AddForwardMACAddress_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_ForwardConfig in;
  struct emptypb_Empty out;
  fdb_FDB_AddForwardMACAddress(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_DeleteForwardMACAddress_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_ForwardConfig in;
  struct emptypb_Empty out;
  fdb_FDB_DeleteForwardMACAddress(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_UpdateForwardMACAddress_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_ForwardConfig in;
  struct emptypb_Empty out;
  fdb_FDB_UpdateForwardMACAddress(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_GetFDBDropTable_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct fdbpb_DropConfig out;
  fdb_FDB_GetFDBDropTable(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_fdb_DropConfig(
      resval,
    &out);
  return res;
}

static status_t intri_fdb_FDB_AddDropMACAddress_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_DropConfig in;
  struct emptypb_Empty out;
  fdb_FDB_AddDropMACAddress(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_DeleteDropMACAddress_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_DropConfig in;
  struct emptypb_Empty out;
  fdb_FDB_DeleteDropMACAddress(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_UpdateDropMACAddress_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_DropConfig in;
  struct emptypb_Empty out;
  fdb_FDB_UpdateDropMACAddress(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_UpdatePortLearningLimit_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_PortLearningLimit in;
  struct emptypb_Empty out;
  fdb_FDB_UpdatePortLearningLimit(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_GetInfo_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct fdbpb_Info out;
  fdb_FDB_GetInfo(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_fdb_Info(
      resval,
    &out);
  return res;
}

static status_t intri_fdb_FDB_GetTable_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct fdbpb_Status out;
  fdb_FDB_GetTable(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_fdb_Status(
      resval,
    &out);
  return res;
}

static status_t intri_fdb_FDB_GetAuthorizedTable_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct fdbpb_Status out;
  fdb_FDB_GetAuthorizedTable(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_fdb_Status(
      resval,
    &out);
  return res;
}

static status_t intri_fdb_FDB_GetSecurityTable_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct fdbpb_Status out;
  fdb_FDB_GetSecurityTable(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_fdb_Status(
      resval,
    &out);
  return res;
}

static status_t intri_fdb_FDB_GetSpecificTable_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_SpecificMac in;
  struct fdbpb_Status out;
  fdb_FDB_GetSpecificTable(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_fdb_Status(
      resval,
    &out);
  return res;
}

static status_t intri_fdb_FDB_RunClearMACTable_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct emptypb_Empty out;
  fdb_FDB_RunClearMACTable(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_RunClearMACTableForInterface_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_FlushOption in;
  struct emptypb_Empty out;
  fdb_FDB_RunClearMACTableForInterface(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_AddEntry_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct eventpb_FDBEntry in;
  struct emptypb_Empty out;
  fdb_FDB_AddEntry(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_DeleteEntry_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct eventpb_FDBEntry in;
  struct emptypb_Empty out;
  fdb_FDB_DeleteEntry(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_UpdateEntry_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct eventpb_FDBEntry in;
  struct emptypb_Empty out;
  fdb_FDB_UpdateEntry(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_UpdatePortOccupied_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct fdbpb_PortOccupied in;
  struct emptypb_Empty out;
  fdb_FDB_UpdatePortOccupied(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_AddMulticastEntry_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct eventpb_FDBEntry in;
  struct emptypb_Empty out;
  fdb_FDB_AddMulticastEntry(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_DeleteMulticastEntry_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct eventpb_FDBEntry in;
  struct emptypb_Empty out;
  fdb_FDB_DeleteMulticastEntry(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_fdb_FDB_UpdateMulticastEntry_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct eventpb_FDBEntry in;
  struct emptypb_Empty out;
  fdb_FDB_UpdateMulticastEntry(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

status_t y_intri_fdb_init(
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

  res = ncxmod_load_module(
      y_M_intri_fdb,
      y_R_intri_fdb,
      &agt_profile->agt_savedevQ,
      &intri_fdb_mod);
  if (res != NO_ERR) {
    return res;
  }

  intri_fdb_GetConfig_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-GetConfig");
  if (intri_fdb_GetConfig_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_GetMACAgingTime_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-GetMACAgingTime");
  if (intri_fdb_GetMACAgingTime_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-GetMACAgingTime",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetMACAgingTime_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_SetMACAgingTime_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-SetMACAgingTime");
  if (intri_fdb_SetMACAgingTime_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-SetMACAgingTime",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_SetMACAgingTime_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_GetFDBForwardTable_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-GetFDBForwardTable");
  if (intri_fdb_GetFDBForwardTable_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-GetFDBForwardTable",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetFDBForwardTable_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_AddForwardMACAddress_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-AddForwardMACAddress");
  if (intri_fdb_AddForwardMACAddress_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-AddForwardMACAddress",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_AddForwardMACAddress_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_DeleteForwardMACAddress_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-DeleteForwardMACAddress");
  if (intri_fdb_DeleteForwardMACAddress_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-DeleteForwardMACAddress",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_DeleteForwardMACAddress_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_UpdateForwardMACAddress_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-UpdateForwardMACAddress");
  if (intri_fdb_UpdateForwardMACAddress_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-UpdateForwardMACAddress",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_UpdateForwardMACAddress_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_GetFDBDropTable_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-GetFDBDropTable");
  if (intri_fdb_GetFDBDropTable_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-GetFDBDropTable",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetFDBDropTable_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_AddDropMACAddress_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-AddDropMACAddress");
  if (intri_fdb_AddDropMACAddress_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-AddDropMACAddress",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_AddDropMACAddress_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_DeleteDropMACAddress_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-DeleteDropMACAddress");
  if (intri_fdb_DeleteDropMACAddress_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-DeleteDropMACAddress",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_DeleteDropMACAddress_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_UpdateDropMACAddress_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-UpdateDropMACAddress");
  if (intri_fdb_UpdateDropMACAddress_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-UpdateDropMACAddress",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_UpdateDropMACAddress_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_UpdatePortLearningLimit_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-UpdatePortLearningLimit");
  if (intri_fdb_UpdatePortLearningLimit_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-UpdatePortLearningLimit",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_UpdatePortLearningLimit_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_GetInfo_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-GetInfo");
  if (intri_fdb_GetInfo_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-GetInfo",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetInfo_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_GetTable_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-GetTable");
  if (intri_fdb_GetTable_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-GetTable",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetTable_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_GetAuthorizedTable_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-GetAuthorizedTable");
  if (intri_fdb_GetAuthorizedTable_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-GetAuthorizedTable",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetAuthorizedTable_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_GetSecurityTable_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-GetSecurityTable");
  if (intri_fdb_GetSecurityTable_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-GetSecurityTable",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetSecurityTable_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_GetSpecificTable_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-GetSpecificTable");
  if (intri_fdb_GetSpecificTable_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-GetSpecificTable",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_GetSpecificTable_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_RunClearMACTable_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-RunClearMACTable");
  if (intri_fdb_RunClearMACTable_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-RunClearMACTable",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_RunClearMACTable_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_RunClearMACTableForInterface_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-RunClearMACTableForInterface");
  if (intri_fdb_RunClearMACTableForInterface_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-RunClearMACTableForInterface",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_RunClearMACTableForInterface_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_AddEntry_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-AddEntry");
  if (intri_fdb_AddEntry_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-AddEntry",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_AddEntry_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_DeleteEntry_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-DeleteEntry");
  if (intri_fdb_DeleteEntry_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-DeleteEntry",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_DeleteEntry_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_UpdateEntry_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-UpdateEntry");
  if (intri_fdb_UpdateEntry_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-UpdateEntry",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_UpdateEntry_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_UpdatePortOccupied_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-UpdatePortOccupied");
  if (intri_fdb_UpdatePortOccupied_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-UpdatePortOccupied",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_UpdatePortOccupied_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_AddMulticastEntry_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-AddMulticastEntry");
  if (intri_fdb_AddMulticastEntry_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-AddMulticastEntry",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_AddMulticastEntry_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_DeleteMulticastEntry_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-DeleteMulticastEntry");
  if (intri_fdb_DeleteMulticastEntry_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-DeleteMulticastEntry",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_DeleteMulticastEntry_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_fdb_UpdateMulticastEntry_obj = ncx_find_object(
      intri_fdb_mod,
      "intri-fdb-UpdateMulticastEntry");
  if (intri_fdb_UpdateMulticastEntry_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_fdb,
      "intri-fdb-UpdateMulticastEntry",
      AGT_RPC_PH_INVOKE,
      intri_fdb_FDB_UpdateMulticastEntry_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

void y_intri_fdb_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-GetConfig");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-GetMACAgingTime");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-SetMACAgingTime");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-GetFDBForwardTable");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-AddForwardMACAddress");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-DeleteForwardMACAddress");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-UpdateForwardMACAddress");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-GetFDBDropTable");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-AddDropMACAddress");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-DeleteDropMACAddress");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-UpdateDropMACAddress");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-UpdatePortLearningLimit");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-GetInfo");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-GetTable");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-GetAuthorizedTable");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-GetSecurityTable");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-GetSpecificTable");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-RunClearMACTable");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-RunClearMACTableForInterface");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-AddEntry");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-DeleteEntry");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-UpdateEntry");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-UpdatePortOccupied");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-AddMulticastEntry");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-DeleteMulticastEntry");

  agt_rpc_unregister_method(
      y_M_intri_fdb,
      "intri-fdb-UpdateMulticastEntry");

}

