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

#include "../../../../../realize/github.com/Intrising/intri-type/common/intri-common@2022-06-24.h"
#include "../../../../../realize/github.com/Intrising/intri-type/core/access/intri-access@2022-06-24.h"
#include "../../../../../realize/github.com/Intrising/intri-type/core/files/intri-files@2022-06-24.h"
#include "../../../../../realize/github.com/Intrising/intri-type/core/userinterface/intri-userinterface@2022-06-24.h"
#include "../../../../../realize/github.com/golang/protobuf/ptypes/empty/intri-empty@2022-06-24.h"

#include "../../../../../genc-trans/github.com/Intrising/intri-type/common/intri-common-trans.h"
#include "../../../../../genc-trans/github.com/Intrising/intri-type/core/access/intri-access-trans.h"
#include "../../../../../genc-trans/github.com/Intrising/intri-type/core/files/intri-files-trans.h"
#include "../../../../../genc-trans/github.com/Intrising/intri-type/core/userinterface/intri-userinterface-trans.h"
#include "../../../../../genc-trans/github.com/Intrising/intri-type/snmp/intri-snmp-trans.h"
#include "../../../../../genc-trans/github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

static ncx_module_t *intri_snmp_mod;


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

  res = agt_load_sil_code(
    y_M_intri_userinterface,
    y_R_intri_userinterface,
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
    y_M_intri_access,
    y_R_intri_access,
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

  res = agt_load_sil_code(
    y_M_intri_files,
    y_R_intri_files,
    true);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }

  res = ncxmod_load_module(
      y_M_intri_snmp,
      y_R_intri_snmp,
      &agt_profile->agt_savedevQ,
      &intri_snmp_mod);
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
}
