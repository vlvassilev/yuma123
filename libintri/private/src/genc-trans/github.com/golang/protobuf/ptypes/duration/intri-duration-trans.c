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

#include "intri-duration-trans.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"


status_t build_to_xml_duration_Duration(
    val_value_t *parentval,
    struct durationpb_Duration *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "Seconds",
      &res);
  if (childval != NULL) {
    val_add_child_sorted(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int64 */
  VAL_LONG(childval) = entry->Seconds;
  childval = agt_make_object(
      parentval->obj,
      "Nanos",
      &res);
  if (childval != NULL) {
    val_add_child_sorted(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->Nanos;
  return res;
}

status_t build_to_priv_duration_Duration(
    val_value_t *parentval,
    struct durationpb_Duration *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "Seconds");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int64 */
    entry->Seconds = VAL_LONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Nanos");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int32 */
    entry->Nanos = VAL_INT(childval);
  }
  return res;
}
