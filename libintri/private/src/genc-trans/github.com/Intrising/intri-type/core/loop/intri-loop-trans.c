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

#include "intri-loop-trans.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"
#include "../../../../../github.com/golang/protobuf/ptypes/timestamp/intri-timestamp-trans.h"

status_t build_to_xml_loop_Config(
    val_value_t *parentval,
    struct looppb_Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "PortList",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  for (int i = 0; i < entry->PortList_Len; i++) {
    val_value_t *listval = NULL;
    listval = agt_make_object(
        childval->obj,
        "PortList_Entry",
        &res);
    if (listval != NULL) {
      val_add_child(listval, childval);
    } else if (res != NO_ERR) {
      return SET_ERROR(res);
    }
    /* message */
    res = build_to_xml_loop_PortConfigEntry(
        listval,
        entry->PortList[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_xml_loop_PortConfigEntry(
    val_value_t *parentval,
    struct looppb_PortConfigEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "IdentifyNo",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_device_InterfaceIdentify(
      childval,
      entry->IdentifyNo);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "Enabled",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->Enabled;
  return res;
}
status_t build_to_xml_loop_PortConfig(
    val_value_t *parentval,
    struct looppb_PortConfig *entry) {
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
    res = build_to_xml_loop_PortConfigEntry(
        listval,
        entry->List[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_xml_loop_StatusEntry(
    val_value_t *parentval,
    struct looppb_StatusEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "IdentifyNo",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_device_InterfaceIdentify(
      childval,
      entry->IdentifyNo);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "LastLoopTs",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_timestamp_Timestamp(
      childval,
      entry->LastLoopTs);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "LoopCount",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->LoopCount;
  childval = agt_make_object(
      parentval->obj,
      "IsLoopCurrent",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->IsLoopCurrent;
  return res;
}
status_t build_to_xml_loop_Status(
    val_value_t *parentval,
    struct looppb_Status *entry) {
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
    res = build_to_xml_loop_StatusEntry(
        listval,
        entry->List[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}

status_t build_to_priv_loop_Config(
    val_value_t *parentval,
    struct looppb_Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "PortList");
  if (childval != NULL && childval->res == NO_ERR) {
    entry->PortList_Len = dlq_count(&childval->v.childQ);
    entry->PortList = malloc((entry->PortList_Len + 1) * sizeof(*entry->PortList));
    unsigned int cnt = 0;
    val_value_t *listval = NULL;
    for (listval = (val_value_t *)dlq_firstEntry(&childval->v.childQ);
         listval != NULL;
         listval = (val_value_t *)dlq_nextEntry(listval)) {
      /* message */
      entry->PortList[cnt] = malloc(sizeof(*(entry->PortList[cnt])));
      res = build_to_priv_loop_PortConfigEntry(
          listval,
          entry->PortList[cnt]);
      if (res != NO_ERR) {
        return SET_ERROR(res);
      }
      cnt++;
    }
  }
  return res;
}
status_t build_to_priv_loop_PortConfigEntry(
    val_value_t *parentval,
    struct looppb_PortConfigEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "IdentifyNo");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->IdentifyNo = malloc(sizeof(*(entry->IdentifyNo)));
    res = build_to_priv_device_InterfaceIdentify(
        childval,
        entry->IdentifyNo);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "Enabled");
  if (childval != NULL && childval->res == NO_ERR) {
    /* bool */
    entry->Enabled = VAL_BOOL(childval);
  }
  return res;
}
status_t build_to_priv_loop_PortConfig(
    val_value_t *parentval,
    struct looppb_PortConfig *entry) {
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
      res = build_to_priv_loop_PortConfigEntry(
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
status_t build_to_priv_loop_StatusEntry(
    val_value_t *parentval,
    struct looppb_StatusEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "IdentifyNo");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->IdentifyNo = malloc(sizeof(*(entry->IdentifyNo)));
    res = build_to_priv_device_InterfaceIdentify(
        childval,
        entry->IdentifyNo);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "LastLoopTs");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->LastLoopTs = malloc(sizeof(*(entry->LastLoopTs)));
    res = build_to_priv_timestamp_Timestamp(
        childval,
        entry->LastLoopTs);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "LoopCount");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int32 */
    entry->LoopCount = VAL_INT(childval);
  }
  childval = val_first_child_name(
      parentval,
      "IsLoopCurrent");
  if (childval != NULL && childval->res == NO_ERR) {
    /* bool */
    entry->IsLoopCurrent = VAL_BOOL(childval);
  }
  return res;
}
status_t build_to_priv_loop_Status(
    val_value_t *parentval,
    struct looppb_Status *entry) {
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
      res = build_to_priv_loop_StatusEntry(
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
