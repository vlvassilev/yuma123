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

#include "intri-monitor-trans.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../github.com/Intrising/intri-type/event/intri-event-trans.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"
status_t build_to_xml_monitor_Config (
    val_value_t *parentval,
    struct monitorpb_Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "DeviceLimitConfig",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_DeviceLimitConfig(
      childval,
    entry->DeviceLimitConfig);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "SystemLimitConfig",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_SystemLimitConfig(
      childval,
    entry->SystemLimitConfig);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_monitor_DeviceLimitConfigEntry (
    val_value_t *parentval,
    struct monitorpb_DeviceLimitConfigEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "MonitorOption",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->MonitorOption;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "LimitOption",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->LimitOption;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Boundary",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_RangeValue(
      childval,
    entry->Boundary);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "ValueOption",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->ValueOption;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Value",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_RangeValue(
      childval,
    entry->Value);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_monitor_DeviceLimitConfig (
    val_value_t *parentval,
    struct monitorpb_DeviceLimitConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "List",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* list */
  for (int i = 0; i < entry->List_Len; i++) {
  val_value_t *listval = NULL;
listval =  agt_make_object(
    childval->obj,
    "List_Entry",
    &res);
if (listval != NULL) {
  val_add_child(listval, childval);
} else if (res != NO_ERR) {
  return SET_ERROR(res);
}
res =  build_to_xml_monitor_DeviceLimitConfigEntry(
    listval,
    entry->List[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  return res;
}

status_t build_to_xml_monitor_SystemLimitConfigEntry (
    val_value_t *parentval,
    struct monitorpb_SystemLimitConfigEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "MonitorOption",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->MonitorOption;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "LimitOption",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->LimitOption;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Boundary",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_RangeValue(
      childval,
    entry->Boundary);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "ValueOption",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->ValueOption;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Value",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_RangeValue(
      childval,
    entry->Value);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_monitor_SystemLimitConfig (
    val_value_t *parentval,
    struct monitorpb_SystemLimitConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "List",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* list */
  for (int i = 0; i < entry->List_Len; i++) {
  val_value_t *listval = NULL;
listval =  agt_make_object(
    childval->obj,
    "List_Entry",
    &res);
if (listval != NULL) {
  val_add_child(listval, childval);
} else if (res != NO_ERR) {
  return SET_ERROR(res);
}
res =  build_to_xml_monitor_SystemLimitConfigEntry(
    listval,
    entry->List[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  return res;
}

status_t build_to_xml_monitor_RangeValue (
    val_value_t *parentval,
    struct monitorpb_RangeValue *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "IntMin",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->IntMin;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "IntMax",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->IntMax;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "FloatMin",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* float */
  VAL_DOUBLE(childval) = entry->FloatMin;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "FloatMax",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* float */
  VAL_DOUBLE(childval) = entry->FloatMax;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Int64Min",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int64 */
  VAL_LONG(childval) = entry->Int64Min;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Int64Max",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int64 */
  VAL_LONG(childval) = entry->Int64Max;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Float64Min",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* double */
  VAL_DOUBLE(childval) = entry->Float64Min;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Float64Max",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* double */
  VAL_DOUBLE(childval) = entry->Float64Max;
  return res;
}

status_t build_to_xml_monitor_DeviceScorllBarValueEntry (
    val_value_t *parentval,
    struct monitorpb_DeviceScorllBarValueEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "MonitorOption",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->MonitorOption;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Value",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_RangeValue(
      childval,
    entry->Value);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_monitor_DeviceScorllBarValue (
    val_value_t *parentval,
    struct monitorpb_DeviceScorllBarValue *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "List",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* list */
  for (int i = 0; i < entry->List_Len; i++) {
  val_value_t *listval = NULL;
listval =  agt_make_object(
    childval->obj,
    "List_Entry",
    &res);
if (listval != NULL) {
  val_add_child(listval, childval);
} else if (res != NO_ERR) {
  return SET_ERROR(res);
}
res =  build_to_xml_monitor_DeviceScorllBarValueEntry(
    listval,
    entry->List[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  return res;
}

status_t build_to_xml_monitor_SystemScorllBarValueEntry (
    val_value_t *parentval,
    struct monitorpb_SystemScorllBarValueEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "MonitorOption",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->MonitorOption;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Value",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_RangeValue(
      childval,
    entry->Value);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_monitor_SystemScorllBarValue (
    val_value_t *parentval,
    struct monitorpb_SystemScorllBarValue *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "List",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* list */
  for (int i = 0; i < entry->List_Len; i++) {
  val_value_t *listval = NULL;
listval =  agt_make_object(
    childval->obj,
    "List_Entry",
    &res);
if (listval != NULL) {
  val_add_child(listval, childval);
} else if (res != NO_ERR) {
  return SET_ERROR(res);
}
res =  build_to_xml_monitor_SystemScorllBarValueEntry(
    listval,
    entry->List[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  return res;
}

status_t build_to_xml_monitor_Status (
    val_value_t *parentval,
    struct monitorpb_Status *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "DeviceStatus",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_DeviceStatus(
      childval,
    entry->DeviceStatus);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "SystemStatus",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_SystemStatus(
      childval,
    entry->SystemStatus);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "LedStatus",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_LEDStatus(
      childval,
    entry->LedStatus);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_monitor_DeviceStatusEntry (
    val_value_t *parentval,
    struct monitorpb_DeviceStatusEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "MonitorOption",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->MonitorOption;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Value",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_DisplayValue(
      childval,
    entry->Value);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "LoggingOption",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->LoggingOption;
  return res;
}

status_t build_to_xml_monitor_DeviceStatus (
    val_value_t *parentval,
    struct monitorpb_DeviceStatus *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "List",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* list */
  for (int i = 0; i < entry->List_Len; i++) {
  val_value_t *listval = NULL;
listval =  agt_make_object(
    childval->obj,
    "List_Entry",
    &res);
if (listval != NULL) {
  val_add_child(listval, childval);
} else if (res != NO_ERR) {
  return SET_ERROR(res);
}
res =  build_to_xml_monitor_DeviceStatusEntry(
    listval,
    entry->List[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  return res;
}

status_t build_to_xml_monitor_SystemStatusEntry (
    val_value_t *parentval,
    struct monitorpb_SystemStatusEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "MonitorOption",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->MonitorOption;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Value",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_DisplayValue(
      childval,
    entry->Value);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "LoggingOption",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->LoggingOption;
  return res;
}

status_t build_to_xml_monitor_SystemStatus (
    val_value_t *parentval,
    struct monitorpb_SystemStatus *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "List",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* list */
  for (int i = 0; i < entry->List_Len; i++) {
  val_value_t *listval = NULL;
listval =  agt_make_object(
    childval->obj,
    "List_Entry",
    &res);
if (listval != NULL) {
  val_add_child(listval, childval);
} else if (res != NO_ERR) {
  return SET_ERROR(res);
}
res =  build_to_xml_monitor_SystemStatusEntry(
    listval,
    entry->List[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  return res;
}

status_t build_to_xml_monitor_DisplayValue (
    val_value_t *parentval,
    struct monitorpb_DisplayValue *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "ValueUnit",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->ValueUnit;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "IntValue",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->IntValue;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "FloatValue",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* float */
  VAL_DOUBLE(childval) = entry->FloatValue;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Int64Value",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int64 */
  VAL_LONG(childval) = entry->Int64Value;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Float64Value",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* double */
  VAL_DOUBLE(childval) = entry->Float64Value;
  return res;
}

status_t build_to_xml_monitor_LEDStatus (
    val_value_t *parentval,
    struct monitorpb_LEDStatus *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "SystemList",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* list */
  for (int i = 0; i < entry->SystemList_Len; i++) {
  val_value_t *listval = NULL;
listval =  agt_make_object(
    childval->obj,
    "SystemList_Entry",
    &res);
if (listval != NULL) {
  val_add_child(listval, childval);
} else if (res != NO_ERR) {
  return SET_ERROR(res);
}
res =  build_to_xml_monitor_SystemLEDStatusEntry(
    listval,
    entry->SystemList[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "PortList",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* list */
  for (int i = 0; i < entry->PortList_Len; i++) {
  val_value_t *listval = NULL;
listval =  agt_make_object(
    childval->obj,
    "PortList_Entry",
    &res);
if (listval != NULL) {
  val_add_child(listval, childval);
} else if (res != NO_ERR) {
  return SET_ERROR(res);
}
res =  build_to_xml_monitor_PortLEDStatusEntry(
    listval,
    entry->PortList[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  return res;
}

status_t build_to_xml_monitor_SystemLEDStatusEntry (
    val_value_t *parentval,
    struct monitorpb_SystemLEDStatusEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "LedType",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->LedType;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "LedState",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_LEDStateEntry(
      childval,
    entry->LedState);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_monitor_PortLEDStatusEntry (
    val_value_t *parentval,
    struct monitorpb_PortLEDStatusEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "IdentifyNo",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_device_InterfaceIdentify(
      childval,
    entry->IdentifyNo);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "IsSfp",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->IsSfp;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "LinkLedState",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_LEDStateEntry(
      childval,
    entry->LinkLedState);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "IsSupportPoELed",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->IsSupportPoELed;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "PoELedState",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_LEDStateEntry(
      childval,
    entry->PoELedState);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "IsSupportSpeedLed",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->IsSupportSpeedLed;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "SpeedLedState",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_monitor_LEDStateEntry(
      childval,
    entry->SpeedLedState);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_monitor_LEDStateEntry (
    val_value_t *parentval,
    struct monitorpb_LEDStateEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "StateOption",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->StateOption;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "ColorOption",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->ColorOption;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "BlinkingIntervalMs",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->BlinkingIntervalMs;
  return res;
}

