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

#include "intri-sfp-trans.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../github.com/Intrising/intri-type/event/intri-event-trans.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

status_t build_to_xml_sfp_Config(
    val_value_t *parentval,
    struct sfppb_Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "LossOfSignalEvent",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->LossOfSignalEvent;
  return res;
}
status_t build_to_xml_sfp_Info(
    val_value_t *parentval,
    struct sfppb_Info *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "List",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  for (int i = 0; i < entry->List_Len; i++) {
    val_value_t *listval = NULL;
    listval = agt_make_object(
        childval->obj,
        "List_Entry",
        &res);
    if (listval != NULL) {
      val_add_child(listval, childval);
    } else if (res != NO_ERR) {
      return SET_ERROR(res);
    }
    /* message */
    res = build_to_xml_event_SFPInfo(
        listval,
        entry->List[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}

status_t build_to_priv_sfp_Config(
    val_value_t *parentval,
    struct sfppb_Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "LossOfSignalEvent");
  if (childval != NULL && childval->res == NO_ERR) {
    /* bool */
    entry->LossOfSignalEvent = VAL_BOOL(childval);
  }
  return res;
}
status_t build_to_priv_sfp_Info(
    val_value_t *parentval,
    struct sfppb_Info *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "List");
  if (childval != NULL && childval->res == NO_ERR) {
    entry->List_Len = dlq_count(&childval->v.childQ);
    entry->List = malloc((entry->List_Len + 1) * sizeof(*entry->List));
    unsigned int cnt = 0;
    val_value_t *listval = NULL;
    for (listval = (val_value_t *)dlq_firstEntry(&childval->v.childQ);
         listval != NULL;
         listval = (val_value_t *)dlq_nextEntry(listval)) {
      /* message */
      entry->List[cnt] = malloc(sizeof(*(entry->List[cnt])));
      res = build_to_priv_event_SFPInfo(
          listval,
          entry->List[cnt]);
      if (res != NO_ERR) {
        return SET_ERROR(res);
      }
      cnt++;
    }
  }
  return res;
}
