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

#include "sercom-cdp@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../../realize/github.com/Intrising/intri-type/device/sercom-device@2022-06-24.h"
#include "../../../../../../realize/github.com/golang/protobuf/ptypes/empty/sercom-empty@2022-06-24.h"

#include "../../../../../../genc-trans/github.com/Intrising/intri-type/core/cdp/intri-cdp-trans.h"
#include "../../../../../../genc-trans/github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_cdp_mod;

static status_t intri_cdp_CDP_GetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct cdppb_Config *out = malloc(sizeof(*out));
  GoString *errstr = malloc(sizeof(*errstr));

  errstr->n = 0;
  cdp_CDP_GetConfig(in, out, errstr);
  if (errstr->n >0) {
    const xmlChar *errorstr = NULL;
    val_value_t *errorval = NULL;
    ncx_errinfo_t *errinfo = ncx_new_errinfo();
    errinfo->error_message = (xmlChar*)(errstr->p);
    res = ERR_NCX_INVALID_VALUE;
    agt_record_error_errinfo(
        scb,
        &msg->mhdr,
        NCX_LAYER_RPC,
        res,
        methnode,
        NCX_NT_NONE,
        errorstr,
        NCX_NT_NONE,
        errorval,
        errinfo);
    log_debug("err= %s", errstr->p);
    free(in);
    free(out);
    free(errstr);
    return res;
  }

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_cdp,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_cdp_Config(outval, out);
  if (res != NO_ERR) {
    free(in);
    free(out);
    free(errstr);
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
  free(errstr);
  return res;
}
static status_t intri_cdp_CDP_SetConfig_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct cdppb_Config *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));
  GoString *errstr = malloc(sizeof(*errstr));

  errstr->n = 0;
  cdp_CDP_GetConfig(out, in, errstr);
  if (errstr->n > 0) {
    const xmlChar *errorstr = NULL;
    val_value_t *errorval = NULL;
    ncx_errinfo_t *errinfo = ncx_new_errinfo();
    errinfo->error_message = (xmlChar*)(errstr->p);
    res = ERR_NCX_INVALID_VALUE;
    agt_record_error_errinfo(
        scb,
        &msg->mhdr,
        NCX_LAYER_RPC,
        res,
        methnode,
        NCX_NT_NONE,
        errorstr,
        NCX_NT_NONE,
        errorval,
        errinfo);
    log_debug("err= %s", errstr->p);
    free(in);
    free(out);
    free(errstr);
    return res;
  }
  res = build_to_priv_cdp_Config(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    free(errstr);
    return SET_ERROR(res);
  }
  errstr->n = 0;
  cdp_CDP_SetConfig(in, out, errstr);
  if (errstr->n >0) {
    const xmlChar *errorstr = NULL;
    val_value_t *errorval = NULL;
    ncx_errinfo_t *errinfo = ncx_new_errinfo();
    errinfo->error_message = (xmlChar*)(errstr->p);
    res = ERR_NCX_INVALID_VALUE;
    agt_record_error_errinfo(
        scb,
        &msg->mhdr,
        NCX_LAYER_RPC,
        res,
        methnode,
        NCX_NT_NONE,
        errorstr,
        NCX_NT_NONE,
        errorval,
        errinfo);
    log_debug("err= %s", errstr->p);
    free(in);
    free(out);
    free(errstr);
    return res;
  }

  free(in);
  free(out);
  free(errstr);
  return res;
}
static status_t intri_cdp_CDP_GetLocalInfo_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct cdppb_LocalInfo *out = malloc(sizeof(*out));
  GoString *errstr = malloc(sizeof(*errstr));

  errstr->n = 0;
  cdp_CDP_GetLocalInfo(in, out, errstr);
  if (errstr->n >0) {
    const xmlChar *errorstr = NULL;
    val_value_t *errorval = NULL;
    ncx_errinfo_t *errinfo = ncx_new_errinfo();
    errinfo->error_message = (xmlChar*)(errstr->p);
    res = ERR_NCX_INVALID_VALUE;
    agt_record_error_errinfo(
        scb,
        &msg->mhdr,
        NCX_LAYER_RPC,
        res,
        methnode,
        NCX_NT_NONE,
        errorstr,
        NCX_NT_NONE,
        errorval,
        errinfo);
    log_debug("err= %s", errstr->p);
    free(in);
    free(out);
    free(errstr);
    return res;
  }

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_cdp,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_cdp_LocalInfo(outval, out);
  if (res != NO_ERR) {
    free(in);
    free(out);
    free(errstr);
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
  free(errstr);
  return res;
}
static status_t intri_cdp_CDP_GetNeighborInfo_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct cdppb_NeighborInfo *out = malloc(sizeof(*out));
  GoString *errstr = malloc(sizeof(*errstr));

  errstr->n = 0;
  cdp_CDP_GetNeighborInfo(in, out, errstr);
  if (errstr->n >0) {
    const xmlChar *errorstr = NULL;
    val_value_t *errorval = NULL;
    ncx_errinfo_t *errinfo = ncx_new_errinfo();
    errinfo->error_message = (xmlChar*)(errstr->p);
    res = ERR_NCX_INVALID_VALUE;
    agt_record_error_errinfo(
        scb,
        &msg->mhdr,
        NCX_LAYER_RPC,
        res,
        methnode,
        NCX_NT_NONE,
        errorstr,
        NCX_NT_NONE,
        errorval,
        errinfo);
    log_debug("err= %s", errstr->p);
    free(in);
    free(out);
    free(errstr);
    return res;
  }

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_cdp,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_cdp_NeighborInfo(outval, out);
  if (res != NO_ERR) {
    free(in);
    free(out);
    free(errstr);
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
  free(errstr);
  return res;
}
static status_t intri_cdp_CDP_GetStatistic_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct emptypb_Empty *in = malloc(sizeof(*in));
  struct cdppb_Statistic *out = malloc(sizeof(*out));
  GoString *errstr = malloc(sizeof(*errstr));

  errstr->n = 0;
  cdp_CDP_GetStatistic(in, out, errstr);
  if (errstr->n >0) {
    const xmlChar *errorstr = NULL;
    val_value_t *errorval = NULL;
    ncx_errinfo_t *errinfo = ncx_new_errinfo();
    errinfo->error_message = (xmlChar*)(errstr->p);
    res = ERR_NCX_INVALID_VALUE;
    agt_record_error_errinfo(
        scb,
        &msg->mhdr,
        NCX_LAYER_RPC,
        res,
        methnode,
        NCX_NT_NONE,
        errorstr,
        NCX_NT_NONE,
        errorval,
        errinfo);
    log_debug("err= %s", errstr->p);
    free(in);
    free(out);
    free(errstr);
    return res;
  }

  obj_template_t *outobj = obj_find_child(
      msg->rpc_method,
      y_M_intri_cdp,
      "output");
  val_value_t *outval = val_new_value();
  val_init_from_template(outval, outobj);

  res = build_to_xml_cdp_Statistic(outval, out);
  if (res != NO_ERR) {
    free(in);
    free(out);
    free(errstr);
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
  free(errstr);
  return res;
}
static status_t intri_cdp_CDP_RunClearStatistic_invoke(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode) {
  status_t res = NO_ERR;
  struct devicepb_PortList *in = malloc(sizeof(*in));
  struct emptypb_Empty *out = malloc(sizeof(*out));
  GoString *errstr = malloc(sizeof(*errstr));

  /* ian: this func has no prefix Update/Set */
  res = build_to_priv_device_PortList(msg->rpc_input, in);
  if (res != NO_ERR) {
    free(in);
    free(out);
    free(errstr);
    return SET_ERROR(res);
  }
  errstr->n = 0;
  cdp_CDP_RunClearStatistic(in, out, errstr);
  if (errstr->n >0) {
    const xmlChar *errorstr = NULL;
    val_value_t *errorval = NULL;
    ncx_errinfo_t *errinfo = ncx_new_errinfo();
    errinfo->error_message = (xmlChar*)(errstr->p);
    res = ERR_NCX_INVALID_VALUE;
    agt_record_error_errinfo(
        scb,
        &msg->mhdr,
        NCX_LAYER_RPC,
        res,
        methnode,
        NCX_NT_NONE,
        errorstr,
        NCX_NT_NONE,
        errorval,
        errinfo);
    log_debug("err= %s", errstr->p);
    free(in);
    free(out);
    free(errstr);
    return res;
  }

  free(in);
  free(out);
  free(errstr);
  return res;
}

status_t y_sercom_cdp_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_cdp)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }
  if (revision && xml_strcmp(revision, y_R_intri_cdp)) {
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

  res = ncxmod_load_module(
      y_M_intri_cdp,
      y_R_intri_cdp,
      &agt_profile->agt_savedevQ,
      &intri_cdp_mod);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_cdp,
      "sercom-cdp-CDP-GetConfig",
      AGT_RPC_PH_INVOKE,
      intri_cdp_CDP_GetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_cdp,
      "sercom-cdp-CDP-SetConfig",
      AGT_RPC_PH_INVOKE,
      intri_cdp_CDP_SetConfig_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_cdp,
      "sercom-cdp-CDP-GetLocalInfo",
      AGT_RPC_PH_INVOKE,
      intri_cdp_CDP_GetLocalInfo_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_cdp,
      "sercom-cdp-CDP-GetNeighborInfo",
      AGT_RPC_PH_INVOKE,
      intri_cdp_CDP_GetNeighborInfo_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_cdp,
      "sercom-cdp-CDP-GetStatistic",
      AGT_RPC_PH_INVOKE,
      intri_cdp_CDP_GetStatistic_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = agt_rpc_register_method(
      y_M_intri_cdp,
      "sercom-cdp-CDP-RunClearStatistic",
      AGT_RPC_PH_INVOKE,
      intri_cdp_CDP_RunClearStatistic_invoke);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  return res;
}

status_t y_sercom_cdp_init2(void) {
  status_t res = NO_ERR;
  return res;
}

void y_sercom_cdp_cleanup(void) {
  agt_rpc_unregister_method(
      y_M_intri_cdp,
      "sercom-cdp-CDP-GetConfig");
  agt_rpc_unregister_method(
      y_M_intri_cdp,
      "sercom-cdp-CDP-SetConfig");
  agt_rpc_unregister_method(
      y_M_intri_cdp,
      "sercom-cdp-CDP-GetLocalInfo");
  agt_rpc_unregister_method(
      y_M_intri_cdp,
      "sercom-cdp-CDP-GetNeighborInfo");
  agt_rpc_unregister_method(
      y_M_intri_cdp,
      "sercom-cdp-CDP-GetStatistic");
  agt_rpc_unregister_method(
      y_M_intri_cdp,
      "sercom-cdp-CDP-RunClearStatistic");
}
