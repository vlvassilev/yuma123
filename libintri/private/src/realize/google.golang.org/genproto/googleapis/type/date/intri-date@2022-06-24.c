
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

#include "intri-date@2022-06-24.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

static ncx_module_t *intri_date_mod;

status_t y_intri_date_init(
    const xmlChar *modname,
    const xmlChar *revision) {
  status_t res = NO_ERR;
  agt_profile_t *agt_profile = agt_get_profile();

  if (xml_strcmp(modname, y_M_intri_date)) {
    return ERR_NCX_UNKNOWN_MODULE;
  }

  if (revision && xml_strcmp(revision, y_R_intri_date)) {
    return ERR_NCX_WRONG_VERSION;
  }

  res = ncxmod_load_module(
      y_M_intri_date,
      y_R_intri_date,
      &agt_profile->agt_savedevQ,
      &intri_date_mod);
  if (res != NO_ERR) {
    return res;
  }

  return res;
}

void y_intri_date_cleanup(void);
