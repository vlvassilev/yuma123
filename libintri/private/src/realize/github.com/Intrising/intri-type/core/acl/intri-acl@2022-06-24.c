
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

#include "intri-acl@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/common/intri-common-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/acl/intri-acl-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_acl_mod;
static obj_template_t *intri_acl_GetConfig_obj;
static obj_template_t *intri_acl_GetInterfaceList_obj;
static obj_template_t *intri_acl_UpdateInterface_obj;
static obj_template_t *intri_acl_UpdateInterfaceList_obj;
static obj_template_t *intri_acl_GetACLList_obj;
static obj_template_t *intri_acl_AddACL_obj;
static obj_template_t *intri_acl_DeleteACL_obj;
static obj_template_t *intri_acl_UpdateACL_obj;
static obj_template_t *intri_acl_GetACEList_obj;
static obj_template_t *intri_acl_AddACE_obj;
static obj_template_t *intri_acl_DeleteACE_obj;
static obj_template_t *intri_acl_UpdateACE_obj;
static obj_template_t *intri_acl_GetBindingList_obj;
static obj_template_t *intri_acl_AddBinding_obj;
static obj_template_t *intri_acl_DeleteBinding_obj;
static obj_template_t *intri_acl_UpdateBinding_obj;
static obj_template_t *intri_acl_GetFlowMirroringList_obj;
static obj_template_t *intri_acl_AddFlowMirroring_obj;
static obj_template_t *intri_acl_DeleteFlowMirroring_obj;
static obj_template_t *intri_acl_UpdateFlowMirroring_obj;

static status_t intri_acl_ACL_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct aclpb_Config out;
  acl_ACL_GetConfig(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_acl_Config(
      resval,
    &out);
  return res;
}

static status_t intri_acl_ACL_GetInterfaceList_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct aclpb_InterfaceList out;
  acl_ACL_GetInterfaceList(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_acl_InterfaceList(
      resval,
    &out);
  return res;
}

static status_t intri_acl_ACL_UpdateInterface_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct aclpb_InterfaceEntry in;
  struct emptypb_Empty out;
  acl_ACL_UpdateInterface(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_acl_ACL_UpdateInterfaceList_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct aclpb_InterfaceList in;
  struct emptypb_Empty out;
  acl_ACL_UpdateInterfaceList(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_acl_ACL_GetACLList_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct aclpb_ACLList out;
  acl_ACL_GetACLList(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_acl_ACLList(
      resval,
    &out);
  return res;
}

static status_t intri_acl_ACL_AddACL_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct aclpb_ACLEntry in;
  struct emptypb_Empty out;
  acl_ACL_AddACL(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_acl_ACL_DeleteACL_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct commonpb_Name in;
  struct emptypb_Empty out;
  acl_ACL_DeleteACL(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_acl_ACL_UpdateACL_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct aclpb_ACLEntry in;
  struct emptypb_Empty out;
  acl_ACL_UpdateACL(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_acl_ACL_GetACEList_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct aclpb_ACEList out;
  acl_ACL_GetACEList(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_acl_ACEList(
      resval,
    &out);
  return res;
}

static status_t intri_acl_ACL_AddACE_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct aclpb_ACEEntry in;
  struct emptypb_Empty out;
  acl_ACL_AddACE(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_acl_ACL_DeleteACE_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct commonpb_Name in;
  struct emptypb_Empty out;
  acl_ACL_DeleteACE(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_acl_ACL_UpdateACE_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct aclpb_ACEEntry in;
  struct emptypb_Empty out;
  acl_ACL_UpdateACE(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_acl_ACL_GetBindingList_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct aclpb_BindingList out;
  acl_ACL_GetBindingList(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_acl_BindingList(
      resval,
    &out);
  return res;
}

static status_t intri_acl_ACL_AddBinding_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct aclpb_BindingEntry in;
  struct emptypb_Empty out;
  acl_ACL_AddBinding(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_acl_ACL_DeleteBinding_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct commonpb_Name in;
  struct emptypb_Empty out;
  acl_ACL_DeleteBinding(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_acl_ACL_UpdateBinding_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct aclpb_BindingEntry in;
  struct emptypb_Empty out;
  acl_ACL_UpdateBinding(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_acl_ACL_GetFlowMirroringList_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty in;
  struct aclpb_FlowMirroringList out;
  acl_ACL_GetFlowMirroringList(&in, &out);
  val_value_t *resval = val_new_value();
  res =  build_to_xml_acl_FlowMirroringList(
      resval,
    &out);
  return res;
}

static status_t intri_acl_ACL_AddFlowMirroring_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct aclpb_FlowMirroringEntry in;
  struct emptypb_Empty out;
  acl_ACL_AddFlowMirroring(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_acl_ACL_DeleteFlowMirroring_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct commonpb_Name in;
  struct emptypb_Empty out;
  acl_ACL_DeleteFlowMirroring(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

static status_t intri_acl_ACL_UpdateFlowMirroring_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct aclpb_FlowMirroringEntry in;
  struct emptypb_Empty out;
  acl_ACL_UpdateFlowMirroring(&in, &out);
  val_value_t *resval = val_new_value();
  return res;
}

status_t y_intri_acl_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_acl)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }

  if (revision && xml_strcmp(revision, y_R_intri_acl)) {
    return ERR_NCX_WRONG_VERSION;
  }

  res = ncxmod_load_module(
      y_M_intri_acl,
      y_R_intri_acl,
      &agt_profile->agt_savedevQ,
      &intri_acl_mod);
  if (res != NO_ERR) {
    return res;
  }

  intri_acl_GetConfig_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-GetConfig");
  if (intri_acl_GetConfig_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_GetInterfaceList_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-GetInterfaceList");
  if (intri_acl_GetInterfaceList_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-GetInterfaceList",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_GetInterfaceList_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_UpdateInterface_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-UpdateInterface");
  if (intri_acl_UpdateInterface_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-UpdateInterface",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_UpdateInterface_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_UpdateInterfaceList_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-UpdateInterfaceList");
  if (intri_acl_UpdateInterfaceList_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-UpdateInterfaceList",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_UpdateInterfaceList_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_GetACLList_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-GetACLList");
  if (intri_acl_GetACLList_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-GetACLList",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_GetACLList_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_AddACL_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-AddACL");
  if (intri_acl_AddACL_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-AddACL",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_AddACL_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_DeleteACL_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-DeleteACL");
  if (intri_acl_DeleteACL_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-DeleteACL",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_DeleteACL_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_UpdateACL_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-UpdateACL");
  if (intri_acl_UpdateACL_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-UpdateACL",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_UpdateACL_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_GetACEList_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-GetACEList");
  if (intri_acl_GetACEList_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-GetACEList",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_GetACEList_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_AddACE_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-AddACE");
  if (intri_acl_AddACE_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-AddACE",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_AddACE_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_DeleteACE_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-DeleteACE");
  if (intri_acl_DeleteACE_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-DeleteACE",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_DeleteACE_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_UpdateACE_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-UpdateACE");
  if (intri_acl_UpdateACE_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-UpdateACE",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_UpdateACE_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_GetBindingList_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-GetBindingList");
  if (intri_acl_GetBindingList_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-GetBindingList",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_GetBindingList_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_AddBinding_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-AddBinding");
  if (intri_acl_AddBinding_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-AddBinding",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_AddBinding_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_DeleteBinding_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-DeleteBinding");
  if (intri_acl_DeleteBinding_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-DeleteBinding",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_DeleteBinding_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_UpdateBinding_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-UpdateBinding");
  if (intri_acl_UpdateBinding_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-UpdateBinding",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_UpdateBinding_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_GetFlowMirroringList_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-GetFlowMirroringList");
  if (intri_acl_GetFlowMirroringList_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-GetFlowMirroringList",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_GetFlowMirroringList_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_AddFlowMirroring_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-AddFlowMirroring");
  if (intri_acl_AddFlowMirroring_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-AddFlowMirroring",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_AddFlowMirroring_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_DeleteFlowMirroring_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-DeleteFlowMirroring");
  if (intri_acl_DeleteFlowMirroring_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-DeleteFlowMirroring",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_DeleteFlowMirroring_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  intri_acl_UpdateFlowMirroring_obj = ncx_find_object(
      intri_acl_mod,
      "intri-acl-UpdateFlowMirroring");
  if (intri_acl_UpdateFlowMirroring_obj == NULL) {
    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
  }

  res = agt_rpc_register_method(
      y_M_intri_acl,
      "intri-acl-UpdateFlowMirroring",
      AGT_RPC_PH_INVOKE,
      intri_acl_ACL_UpdateFlowMirroring_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

void y_intri_acl_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-GetConfig");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-GetInterfaceList");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-UpdateInterface");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-UpdateInterfaceList");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-GetACLList");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-AddACL");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-DeleteACL");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-UpdateACL");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-GetACEList");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-AddACE");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-DeleteACE");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-UpdateACE");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-GetBindingList");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-AddBinding");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-DeleteBinding");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-UpdateBinding");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-GetFlowMirroringList");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-AddFlowMirroring");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-DeleteFlowMirroring");

  agt_rpc_unregister_method(
      y_M_intri_acl,
      "intri-acl-UpdateFlowMirroring");

}

