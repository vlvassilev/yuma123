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

#include "intri-snmp@2022-06-24.h"
#include "../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../genc-trans/github.com/Intrising/intri-type/common/intri-common-trans.h"
#include "../../../../../genc-trans/github.com/Intrising/intri-type/core/access/intri-access-trans.h"
#include "../../../../../genc-trans/github.com/Intrising/intri-type/core/files/intri-files-trans.h"
#include "../../../../../genc-trans/github.com/Intrising/intri-type/core/userinterface/intri-userinterface-trans.h"
#include "../../../../../genc-trans/github.com/Intrising/intri-type/snmp/intri-snmp-trans.h"
#include "../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_snmp_mod;

static status_t intri_snmp_SNMP_SetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct userinterfacepb_SNMPConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_userinterface_SNMPConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  snmp_SNMP_SetConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_snmp_SNMP_SetUsersConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_UsersConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_access_UsersConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  snmp_SNMP_SetUsersConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_snmp_SNMP_SetGroupsConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_GroupsConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_access_GroupsConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  snmp_SNMP_SetGroupsConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_snmp_SNMP_RunRestartSNMPServer_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct commonpb_Confirm *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_common_Confirm(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  snmp_SNMP_RunRestartSNMPServer(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_snmp_SNMP_ActivateAgentCertificate_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct filespb_CertificateID *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_files_CertificateID(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  snmp_SNMP_ActivateAgentCertificate(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_snmp_SNMP_ActivateManagerCertificate_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct filespb_CertificateUserIDList *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_files_CertificateUserIDList(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  snmp_SNMP_ActivateManagerCertificate(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_snmp_SNMP_DeactivateManagerCertificate_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct filespb_CertificateUserID *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_files_CertificateUserID(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  snmp_SNMP_DeactivateManagerCertificate(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_snmp_SNMP_GetEngineInfo_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct snmppb_EngineInfo *out = malloc(sizeof(*out));

  snmp_SNMP_GetEngineInfo(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_snmp,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_snmp_EngineInfo(outval, out);
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

status_t y_intri_snmp_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_snmp)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }
  if (revision && xml_strcmp(revision, y_R_intri_snmp)) {
    return ERR_NCX_WRONG_VERSION;
  }

  res = ncxmod_load_module(
      y_M_intri_snmp,
      y_R_intri_snmp,
      &agt_profile->agt_savedevQ,
      &intri_snmp_mod);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-SetConfig",
      AGT_RPC_PH_INVOKE,
      intri_snmp_SNMP_SetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-SetUsersConfig",
      AGT_RPC_PH_INVOKE,
      intri_snmp_SNMP_SetUsersConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-SetGroupsConfig",
      AGT_RPC_PH_INVOKE,
      intri_snmp_SNMP_SetGroupsConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-RunRestartSNMPServer",
      AGT_RPC_PH_INVOKE,
      intri_snmp_SNMP_RunRestartSNMPServer_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-ActivateAgentCertificate",
      AGT_RPC_PH_INVOKE,
      intri_snmp_SNMP_ActivateAgentCertificate_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-ActivateManagerCertificate",
      AGT_RPC_PH_INVOKE,
      intri_snmp_SNMP_ActivateManagerCertificate_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-DeactivateManagerCertificate",
      AGT_RPC_PH_INVOKE,
      intri_snmp_SNMP_DeactivateManagerCertificate_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-GetEngineInfo",
      AGT_RPC_PH_INVOKE,
      intri_snmp_SNMP_GetEngineInfo_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

status_t y_intri_snmp_init2(void) {
  status_t res = NO_ERR;
  return res;
}

void y_intri_snmp_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-SetConfig");
  agt_rpc_unregister_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-SetUsersConfig");
  agt_rpc_unregister_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-SetGroupsConfig");
  agt_rpc_unregister_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-RunRestartSNMPServer");
  agt_rpc_unregister_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-ActivateAgentCertificate");
  agt_rpc_unregister_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-ActivateManagerCertificate");
  agt_rpc_unregister_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-DeactivateManagerCertificate");
  agt_rpc_unregister_method(
      y_M_intri_snmp,
      "intri-snmp-SNMP-GetEngineInfo");
}
