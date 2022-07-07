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

#include "intri-access@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../../realize/github.com/Intrising/intri-type/common/intri-common@2022-06-24.h"
#include "../../../../../../realize/github.com/Intrising/intri-type/event/intri-event@2022-06-24.h"
#include "../../../../../../realize/github.com/golang/protobuf/ptypes/empty/intri-empty@2022-06-24.h"

#include "../../../../../../genc-trans/github.com/Intrising/intri-type/common/intri-common-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/access/intri-access-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/event/intri-event-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_access_mod;

static status_t intri_access_Access_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct accesspb_Config *out = malloc(sizeof(*out));

  access_Access_GetConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_access,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_access_Config(outval, out);
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
static status_t intri_access_Access_GetAuthenticationConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct accesspb_AuthenticationConfig *out = malloc(sizeof(*out));

  access_Access_GetAuthenticationConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_access,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_access_AuthenticationConfig(outval, out);
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
static status_t intri_access_Access_SetAuthenticationConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_AuthenticationConfig *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  access_Access_GetAuthenticationConfig(out, in);
  res = build_to_priv_access_AuthenticationConfig(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  access_Access_SetAuthenticationConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_Login_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_LoginRequest *in = malloc(sizeof(*in));
  struct accesspb_Token *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_access_LoginRequest(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  access_Access_Login(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_access,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_access_Token(outval, out);
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
static status_t intri_access_Access_Logout_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  access_Access_Logout(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_PAMLogin_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_LoginRequest *in = malloc(sizeof(*in));
  struct eventpb_LoginParameter *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_access_LoginRequest(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  access_Access_PAMLogin(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_access,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_event_LoginParameter(outval, out);
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
static status_t intri_access_Access_GetUsers_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct accesspb_UsersConfig *out = malloc(sizeof(*out));

  access_Access_GetUsers(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_access,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_access_UsersConfig(outval, out);
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
static status_t intri_access_Access_AddUser_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_UserEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_access_UserEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  access_Access_AddUser(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_DeleteUser_invoke(
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
  access_Access_DeleteUser(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_UpdateUser_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_UserEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_access_UserEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  access_Access_UpdateUser(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_RunEncryptPassword_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_Password *in = malloc(sizeof(*in));
  struct accesspb_EncryptedPassword *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_access_Password(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  access_Access_RunEncryptPassword(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_access,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_access_EncryptedPassword(outval, out);
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
static status_t intri_access_Access_RunUserEnterPassword_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_UserPassword *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_access_UserPassword(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  access_Access_RunUserEnterPassword(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_RunUserEnterSNMPV3AuthPassword_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_UserPassword *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_access_UserPassword(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  access_Access_RunUserEnterSNMPV3AuthPassword(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_RunUserEnterSNMPV3PrivacyPassword_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_UserPassword *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_access_UserPassword(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  access_Access_RunUserEnterSNMPV3PrivacyPassword(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_GetGroups_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct accesspb_GroupsConfig *out = malloc(sizeof(*out));

  access_Access_GetGroups(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_access,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_access_GroupsConfig(outval, out);
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
static status_t intri_access_Access_AddGroup_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_GroupEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_access_GroupEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  access_Access_AddGroup(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_DeleteGroup_invoke(
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
  access_Access_DeleteGroup(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_UpdateGroup_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_GroupEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_access_GroupEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  access_Access_UpdateGroup(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_GetAuthenticatorServerConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct accesspb_AuthenticationServersConfig *out = malloc(sizeof(*out));

  access_Access_GetAuthenticatorServerConfig(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_access,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_access_AuthenticationServersConfig(outval, out);
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
static status_t intri_access_Access_AddAuthenticatorServerConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_AuthenticationServerEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_access_AuthenticationServerEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  access_Access_AddAuthenticatorServerConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_DeleteAuthenticatorServerConfig_invoke(
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
  access_Access_DeleteAuthenticatorServerConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_UpdateAuthenticatorServerConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_AuthenticationServerEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: the Get func has input */
  res = build_to_priv_access_AuthenticationServerEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  access_Access_UpdateAuthenticatorServerConfig(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_GetRestrictions_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct accesspb_RestrictionsConfig *out = malloc(sizeof(*out));

  access_Access_GetRestrictions(in, out);

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_access,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_access_RestrictionsConfig(outval, out);
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
static status_t intri_access_Access_AddRestriction_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_RestrictionEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_access_RestrictionEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  access_Access_AddRestriction(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_DeleteRestriction_invoke(
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
  access_Access_DeleteRestriction(in, out);

  free(in);
  free(out);
  return res;
}
static status_t intri_access_Access_UpdateRestriction_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct accesspb_RestrictionEntry *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));

  /* ian: has no Get func */
  res = build_to_priv_access_RestrictionEntry(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    return SET_ERROR(res);
  }
  access_Access_UpdateRestriction(in, out);

  free(in);
  free(out);
  return res;
}

status_t y_intri_access_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_access)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }
  if (revision && xml_strcmp(revision, y_R_intri_access)) {
    return ERR_NCX_WRONG_VERSION;
  }

  res = agt_load_sil_code(
    y_M_intri_event,
    y_R_intri_event,
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
    y_M_intri_common,
    y_R_intri_common,
    true);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = ncxmod_load_module(
      y_M_intri_access,
      y_R_intri_access,
      &agt_profile->agt_savedevQ,
      &intri_access_mod);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-GetAuthenticationConfig",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_GetAuthenticationConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-SetAuthenticationConfig",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_SetAuthenticationConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-Login",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_Login_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-Logout",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_Logout_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-PAMLogin",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_PAMLogin_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-GetUsers",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_GetUsers_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-AddUser",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_AddUser_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-DeleteUser",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_DeleteUser_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-UpdateUser",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_UpdateUser_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-RunEncryptPassword",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_RunEncryptPassword_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-RunUserEnterPassword",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_RunUserEnterPassword_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-RunUserEnterSNMPV3AuthPassword",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_RunUserEnterSNMPV3AuthPassword_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-RunUserEnterSNMPV3PrivacyPassword",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_RunUserEnterSNMPV3PrivacyPassword_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-GetGroups",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_GetGroups_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-AddGroup",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_AddGroup_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-DeleteGroup",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_DeleteGroup_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-UpdateGroup",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_UpdateGroup_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-GetAuthenticatorServerConfig",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_GetAuthenticatorServerConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-AddAuthenticatorServerConfig",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_AddAuthenticatorServerConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-DeleteAuthenticatorServerConfig",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_DeleteAuthenticatorServerConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-UpdateAuthenticatorServerConfig",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_UpdateAuthenticatorServerConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-GetRestrictions",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_GetRestrictions_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-AddRestriction",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_AddRestriction_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-DeleteRestriction",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_DeleteRestriction_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_access,
      "intri-access-Access-UpdateRestriction",
      AGT_RPC_PH_INVOKE,
      intri_access_Access_UpdateRestriction_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

status_t y_intri_access_init2(void) {
  status_t res = NO_ERR;
  return res;
}

void y_intri_access_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-GetConfig");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-GetAuthenticationConfig");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-SetAuthenticationConfig");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-Login");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-Logout");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-PAMLogin");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-GetUsers");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-AddUser");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-DeleteUser");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-UpdateUser");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-RunEncryptPassword");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-RunUserEnterPassword");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-RunUserEnterSNMPV3AuthPassword");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-RunUserEnterSNMPV3PrivacyPassword");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-GetGroups");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-AddGroup");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-DeleteGroup");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-UpdateGroup");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-GetAuthenticatorServerConfig");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-AddAuthenticatorServerConfig");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-DeleteAuthenticatorServerConfig");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-UpdateAuthenticatorServerConfig");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-GetRestrictions");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-AddRestriction");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-DeleteRestriction");
  agt_rpc_unregister_method(
      y_M_intri_access,
      "intri-access-Access-UpdateRestriction");
}
