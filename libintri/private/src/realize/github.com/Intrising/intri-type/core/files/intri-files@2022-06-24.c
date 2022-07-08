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

#include "intri-files@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../../realize/github.com/Intrising/intri-type/common/intri-common@2022-06-24.h"
#include "../../../../../../realize/github.com/golang/protobuf/ptypes/empty/intri-empty@2022-06-24.h"

#include "../../../../../../genc-trans/github.com/Intrising/intri-type/common/intri-common-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/files/intri-files-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_files_mod;

static status_t intri_files_Files_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct filespb_Config *out = malloc(sizeof(*out));

  files_Files_GetConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_files,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_files_Config(outval, out);
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
static status_t intri_files_Files_GetFTPServerEnabled_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct commonpb_Enabled *out = malloc(sizeof(*out));

  files_Files_GetFTPServerEnabled(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_files,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_common_Enabled(outval, out);
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
static status_t intri_files_Files_SetFTPServerEnabled_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct commonpb_Enabled *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  files_Files_GetFTPServerEnabled(out, in);
  res = build_to_priv_common_Enabled(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  files_Files_SetFTPServerEnabled(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_files_Files_GetActivateCertificate_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct filespb_ActivateCertificate *out = malloc(sizeof(*out));

  files_Files_GetActivateCertificate(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_files,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_files_ActivateCertificate(outval, out);
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
static status_t intri_files_Files_SetActivateCertificateForWeb_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct filespb_CertificateID *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_files_CertificateID(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  files_Files_SetActivateCertificateForWeb(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_files_Files_SetActivateCertificateForSNMP_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct filespb_CertificateID *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_files_CertificateID(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  files_Files_SetActivateCertificateForSNMP(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_files_Files_AddActivateCertificateForSNMPManager_invoke(
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
  files_Files_AddActivateCertificateForSNMPManager(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_files_Files_DeleteActivateCertificateForSNMPManager_invoke(
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
  files_Files_DeleteActivateCertificateForSNMPManager(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_files_Files_UpdateActivateCertificateForSNMPManager_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct filespb_CertificateUserID *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_files_CertificateUserID(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  files_Files_UpdateActivateCertificateForSNMPManager(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_files_Files_RunImportCertificate_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct filespb_CertificateData *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_files_CertificateData(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  files_Files_RunImportCertificate(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_files_Files_RunExportCertificate_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct filespb_CertificateData *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_files_CertificateData(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  files_Files_RunExportCertificate(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_files_Files_RunRemoveCertificate_invoke(
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
  files_Files_RunRemoveCertificate(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_files_Files_GetCertificateList_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct filespb_CertificateIDList *out = malloc(sizeof(*out));

  files_Files_GetCertificateList(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_files,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_files_CertificateIDList(outval, out);
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

status_t y_intri_files_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_files)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }
  if (revision && xml_strcmp(revision, y_R_intri_files)) {
    return ERR_NCX_WRONG_VERSION;
  }

  res = agt_load_sil_code(
    y_M_intri_empty,
    y_R_intri_empty,
    true);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_load_sil_code(
    y_M_intri_common,
    y_R_intri_common,
    true);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = ncxmod_load_module(
      y_M_intri_files,
      y_R_intri_files,
      &agt_profile->agt_savedevQ,
      &intri_files_mod);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_files,
      "intri-files-Files-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_files_Files_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_files,
      "intri-files-Files-GetFTPServerEnabled",
      AGT_RPC_PH_INVOKE,
      intri_files_Files_GetFTPServerEnabled_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_files,
      "intri-files-Files-SetFTPServerEnabled",
      AGT_RPC_PH_INVOKE,
      intri_files_Files_SetFTPServerEnabled_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_files,
      "intri-files-Files-GetActivateCertificate",
      AGT_RPC_PH_INVOKE,
      intri_files_Files_GetActivateCertificate_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_files,
      "intri-files-Files-SetActivateCertificateForWeb",
      AGT_RPC_PH_INVOKE,
      intri_files_Files_SetActivateCertificateForWeb_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_files,
      "intri-files-Files-SetActivateCertificateForSNMP",
      AGT_RPC_PH_INVOKE,
      intri_files_Files_SetActivateCertificateForSNMP_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_files,
      "intri-files-Files-AddActivateCertificateForSNMPManager",
      AGT_RPC_PH_INVOKE,
      intri_files_Files_AddActivateCertificateForSNMPManager_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_files,
      "intri-files-Files-DeleteActivateCertificateForSNMPManager",
      AGT_RPC_PH_INVOKE,
      intri_files_Files_DeleteActivateCertificateForSNMPManager_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_files,
      "intri-files-Files-UpdateActivateCertificateForSNMPManager",
      AGT_RPC_PH_INVOKE,
      intri_files_Files_UpdateActivateCertificateForSNMPManager_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_files,
      "intri-files-Files-RunImportCertificate",
      AGT_RPC_PH_INVOKE,
      intri_files_Files_RunImportCertificate_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_files,
      "intri-files-Files-RunExportCertificate",
      AGT_RPC_PH_INVOKE,
      intri_files_Files_RunExportCertificate_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_files,
      "intri-files-Files-RunRemoveCertificate",
      AGT_RPC_PH_INVOKE,
      intri_files_Files_RunRemoveCertificate_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_files,
      "intri-files-Files-GetCertificateList",
      AGT_RPC_PH_INVOKE,
      intri_files_Files_GetCertificateList_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

status_t y_intri_files_init2(void) {
  status_t res = NO_ERR;
  return res;
}

void y_intri_files_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_files,
      "intri-files-Files-GetConfig");
  agt_rpc_unregister_method(
      y_M_intri_files,
      "intri-files-Files-GetFTPServerEnabled");
  agt_rpc_unregister_method(
      y_M_intri_files,
      "intri-files-Files-SetFTPServerEnabled");
  agt_rpc_unregister_method(
      y_M_intri_files,
      "intri-files-Files-GetActivateCertificate");
  agt_rpc_unregister_method(
      y_M_intri_files,
      "intri-files-Files-SetActivateCertificateForWeb");
  agt_rpc_unregister_method(
      y_M_intri_files,
      "intri-files-Files-SetActivateCertificateForSNMP");
  agt_rpc_unregister_method(
      y_M_intri_files,
      "intri-files-Files-AddActivateCertificateForSNMPManager");
  agt_rpc_unregister_method(
      y_M_intri_files,
      "intri-files-Files-DeleteActivateCertificateForSNMPManager");
  agt_rpc_unregister_method(
      y_M_intri_files,
      "intri-files-Files-UpdateActivateCertificateForSNMPManager");
  agt_rpc_unregister_method(
      y_M_intri_files,
      "intri-files-Files-RunImportCertificate");
  agt_rpc_unregister_method(
      y_M_intri_files,
      "intri-files-Files-RunExportCertificate");
  agt_rpc_unregister_method(
      y_M_intri_files,
      "intri-files-Files-RunRemoveCertificate");
  agt_rpc_unregister_method(
      y_M_intri_files,
      "intri-files-Files-GetCertificateList");
}
