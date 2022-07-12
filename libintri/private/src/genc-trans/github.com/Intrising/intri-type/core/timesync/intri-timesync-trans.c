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

#include "intri-timesync-trans.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

status_t build_to_xml_timesync_Config(
    val_value_t *parentval,
    struct timesyncpb_Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "GNSS",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_timesync_GNSSConfig(
      childval,
      entry->GNSS);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "SyncE",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_timesync_SyncEConfig(
      childval,
      entry->SyncE);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "SyncSource",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_timesync_SyncSourceConfig(
      childval,
      entry->SyncSource);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "Reference",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_timesync_ReferenceOutput(
      childval,
      entry->Reference);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}
status_t build_to_xml_timesync_GNSSConfig(
    val_value_t *parentval,
    struct timesyncpb_GNSSConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
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
status_t build_to_xml_timesync_SyncEConfig(
    val_value_t *parentval,
    struct timesyncpb_SyncEConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "Mode",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  switch (entry->Mode) {
    case timesyncpb_SyncEModeTypeOptions_SYNCE_MODE_TYPE_DISABLED:
      enum_str = "SYNCE_MODE_TYPE_DISABLED";
      break;
    case timesyncpb_SyncEModeTypeOptions_SYNCE_MODE_TYPE_STATIC:
      enum_str = "SYNCE_MODE_TYPE_STATIC";
      break;
    case timesyncpb_SyncEModeTypeOptions_SYNCE_MODE_TYPE_ESMC:
      enum_str = "SYNCE_MODE_TYPE_ESMC";
      break;
  }
  VAL_ENUM_NAME(childval) = enum_str;
  childval = agt_make_object(
      parentval->obj,
      "ReferenceIdentifyNo",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_device_InterfaceIdentify(
      childval,
      entry->ReferenceIdentifyNo);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}
status_t build_to_xml_timesync_SyncSourceConfig(
    val_value_t *parentval,
    struct timesyncpb_SyncSourceConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "Source",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  switch (entry->Source) {
    case timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_GNSS:
      enum_str = "SYNC_SOURCE_TYPE_GNSS";
      break;
    case timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_SYNCE:
      enum_str = "SYNC_SOURCE_TYPE_SYNCE";
      break;
    case timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_10MHZ_INPUT:
      enum_str = "SYNC_SOURCE_TYPE_10MHZ_INPUT";
      break;
    case timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_1PPS_INPUT:
      enum_str = "SYNC_SOURCE_TYPE_1PPS_INPUT";
      break;
    case timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_BITS_INPUT:
      enum_str = "SYNC_SOURCE_TYPE_BITS_INPUT";
      break;
    case timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_PTP:
      enum_str = "SYNC_SOURCE_TYPE_PTP";
      break;
  }
  VAL_ENUM_NAME(childval) = enum_str;
  return res;
}
status_t build_to_xml_timesync_ReferenceOutput(
    val_value_t *parentval,
    struct timesyncpb_ReferenceOutput *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "Tod",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_timesync_ToDConfig(
      childval,
      entry->Tod);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}
status_t build_to_xml_timesync_ToDConfig(
    val_value_t *parentval,
    struct timesyncpb_ToDConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
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
  childval = agt_make_object(
      parentval->obj,
      "MessageType",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  switch (entry->MessageType) {
    case timesyncpb_ToDMessageTypeOptions_TOD_MESSAGE_TYPE_NMEA_GPZDA:
      enum_str = "TOD_MESSAGE_TYPE_NMEA_GPZDA";
      break;
  }
  VAL_ENUM_NAME(childval) = enum_str;
  return res;
}
status_t build_to_xml_timesync_GNSStatus(
    val_value_t *parentval,
    struct timesyncpb_GNSStatus *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "State",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  switch (entry->State) {
    case timesyncpb_GNSSStateTypeOptions_GNSS_STATE_TYPE_DISABLE:
      enum_str = "GNSS_STATE_TYPE_DISABLE";
      break;
    case timesyncpb_GNSSStateTypeOptions_GNSS_STATE_TYPE_SYNC:
      enum_str = "GNSS_STATE_TYPE_SYNC";
      break;
    case timesyncpb_GNSSStateTypeOptions_GNSS_STATE_TYPE_TRACKING:
      enum_str = "GNSS_STATE_TYPE_TRACKING";
      break;
  }
  VAL_ENUM_NAME(childval) = enum_str;
  childval = agt_make_object(
      parentval->obj,
      "Longitude",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->Longitude;
  childval = agt_make_object(
      parentval->obj,
      "Latitude",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->Latitude;
  childval = agt_make_object(
      parentval->obj,
      "DateTime",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->DateTime;
  return res;
}
status_t build_to_xml_timesync_SyncEStatus(
    val_value_t *parentval,
    struct timesyncpb_SyncEStatus *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "ReferenceIdentifyNo",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_device_InterfaceIdentify(
      childval,
      entry->ReferenceIdentifyNo);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "Signal",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  switch (entry->Signal) {
    case timesyncpb_SignalTypeOptions_SIGNAL_TYPE_OK:
      enum_str = "SIGNAL_TYPE_OK";
      break;
    case timesyncpb_SignalTypeOptions_SIGNAL_TYPE_LOSS:
      enum_str = "SIGNAL_TYPE_LOSS";
      break;
  }
  VAL_ENUM_NAME(childval) = enum_str;
  childval = agt_make_object(
      parentval->obj,
      "LockStatus",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  switch (entry->LockStatus) {
    case timesyncpb_LockStatusTypeOptions_LOCK_STATUS_TYPE_FREERUN:
      enum_str = "LOCK_STATUS_TYPE_FREERUN";
      break;
    case timesyncpb_LockStatusTypeOptions_LOCK_STATUS_TYPE_LOCK_ACQUISITION:
      enum_str = "LOCK_STATUS_TYPE_LOCK_ACQUISITION";
      break;
    case timesyncpb_LockStatusTypeOptions_LOCK_STATUS_TYPE_LOCKED:
      enum_str = "LOCK_STATUS_TYPE_LOCKED";
      break;
    case timesyncpb_LockStatusTypeOptions_LOCK_STATUS_TYPE_HOLDOVER:
      enum_str = "LOCK_STATUS_TYPE_HOLDOVER";
      break;
  }
  VAL_ENUM_NAME(childval) = enum_str;
  return res;
}
status_t build_to_xml_timesync_SyncSourceStatus(
    val_value_t *parentval,
    struct timesyncpb_SyncSourceStatus *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "LockStatus",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  switch (entry->LockStatus) {
    case timesyncpb_LockStatusTypeOptions_LOCK_STATUS_TYPE_FREERUN:
      enum_str = "LOCK_STATUS_TYPE_FREERUN";
      break;
    case timesyncpb_LockStatusTypeOptions_LOCK_STATUS_TYPE_LOCK_ACQUISITION:
      enum_str = "LOCK_STATUS_TYPE_LOCK_ACQUISITION";
      break;
    case timesyncpb_LockStatusTypeOptions_LOCK_STATUS_TYPE_LOCKED:
      enum_str = "LOCK_STATUS_TYPE_LOCKED";
      break;
    case timesyncpb_LockStatusTypeOptions_LOCK_STATUS_TYPE_HOLDOVER:
      enum_str = "LOCK_STATUS_TYPE_HOLDOVER";
      break;
  }
  VAL_ENUM_NAME(childval) = enum_str;
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
    res = build_to_xml_timesync_SyncSourceInputStatusEntry(
        listval,
        entry->List[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_xml_timesync_SyncSourceInputStatusEntry(
    val_value_t *parentval,
    struct timesyncpb_SyncSourceInputStatusEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "Source",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  switch (entry->Source) {
    case timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_GNSS:
      enum_str = "SYNC_SOURCE_TYPE_GNSS";
      break;
    case timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_SYNCE:
      enum_str = "SYNC_SOURCE_TYPE_SYNCE";
      break;
    case timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_10MHZ_INPUT:
      enum_str = "SYNC_SOURCE_TYPE_10MHZ_INPUT";
      break;
    case timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_1PPS_INPUT:
      enum_str = "SYNC_SOURCE_TYPE_1PPS_INPUT";
      break;
    case timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_BITS_INPUT:
      enum_str = "SYNC_SOURCE_TYPE_BITS_INPUT";
      break;
    case timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_PTP:
      enum_str = "SYNC_SOURCE_TYPE_PTP";
      break;
  }
  VAL_ENUM_NAME(childval) = enum_str;
  childval = agt_make_object(
      parentval->obj,
      "Signal",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  switch (entry->Signal) {
    case timesyncpb_SignalTypeOptions_SIGNAL_TYPE_OK:
      enum_str = "SIGNAL_TYPE_OK";
      break;
    case timesyncpb_SignalTypeOptions_SIGNAL_TYPE_LOSS:
      enum_str = "SIGNAL_TYPE_LOSS";
      break;
  }
  VAL_ENUM_NAME(childval) = enum_str;
  return res;
}

status_t build_to_priv_timesync_Config(
    val_value_t *parentval,
    struct timesyncpb_Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "GNSS");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->GNSS = malloc(sizeof(*(entry->GNSS)));
    res = build_to_priv_timesync_GNSSConfig(
        childval,
        entry->GNSS);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "SyncE");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->SyncE = malloc(sizeof(*(entry->SyncE)));
    res = build_to_priv_timesync_SyncEConfig(
        childval,
        entry->SyncE);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "SyncSource");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->SyncSource = malloc(sizeof(*(entry->SyncSource)));
    res = build_to_priv_timesync_SyncSourceConfig(
        childval,
        entry->SyncSource);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "Reference");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->Reference = malloc(sizeof(*(entry->Reference)));
    res = build_to_priv_timesync_ReferenceOutput(
        childval,
        entry->Reference);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_priv_timesync_GNSSConfig(
    val_value_t *parentval,
    struct timesyncpb_GNSSConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "Enabled");
  if (childval != NULL && childval->res == NO_ERR) {
    /* bool */
    entry->Enabled = VAL_BOOL(childval);
  }
  return res;
}
status_t build_to_priv_timesync_SyncEConfig(
    val_value_t *parentval,
    struct timesyncpb_SyncEConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "Mode");
  if (childval != NULL && childval->res == NO_ERR) {
    /* enum */
    entry->Mode = VAL_ENUM(childval);
  }
  childval = val_first_child_name(
      parentval,
      "ReferenceIdentifyNo");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->ReferenceIdentifyNo = malloc(sizeof(*(entry->ReferenceIdentifyNo)));
    res = build_to_priv_device_InterfaceIdentify(
        childval,
        entry->ReferenceIdentifyNo);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_priv_timesync_SyncSourceConfig(
    val_value_t *parentval,
    struct timesyncpb_SyncSourceConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "Source");
  if (childval != NULL && childval->res == NO_ERR) {
    /* enum */
    entry->Source = VAL_ENUM(childval);
  }
  return res;
}
status_t build_to_priv_timesync_ReferenceOutput(
    val_value_t *parentval,
    struct timesyncpb_ReferenceOutput *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "Tod");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->Tod = malloc(sizeof(*(entry->Tod)));
    res = build_to_priv_timesync_ToDConfig(
        childval,
        entry->Tod);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_priv_timesync_ToDConfig(
    val_value_t *parentval,
    struct timesyncpb_ToDConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "Enabled");
  if (childval != NULL && childval->res == NO_ERR) {
    /* bool */
    entry->Enabled = VAL_BOOL(childval);
  }
  childval = val_first_child_name(
      parentval,
      "MessageType");
  if (childval != NULL && childval->res == NO_ERR) {
    /* enum */
    entry->MessageType = VAL_ENUM(childval);
  }
  return res;
}
status_t build_to_priv_timesync_GNSStatus(
    val_value_t *parentval,
    struct timesyncpb_GNSStatus *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "State");
  if (childval != NULL && childval->res == NO_ERR) {
    /* enum */
    entry->State = VAL_ENUM(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Longitude");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->Longitude = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Latitude");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->Latitude = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "DateTime");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->DateTime = VAL_STRING(childval);
  }
  return res;
}
status_t build_to_priv_timesync_SyncEStatus(
    val_value_t *parentval,
    struct timesyncpb_SyncEStatus *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "ReferenceIdentifyNo");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->ReferenceIdentifyNo = malloc(sizeof(*(entry->ReferenceIdentifyNo)));
    res = build_to_priv_device_InterfaceIdentify(
        childval,
        entry->ReferenceIdentifyNo);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "Signal");
  if (childval != NULL && childval->res == NO_ERR) {
    /* enum */
    entry->Signal = VAL_ENUM(childval);
  }
  childval = val_first_child_name(
      parentval,
      "LockStatus");
  if (childval != NULL && childval->res == NO_ERR) {
    /* enum */
    entry->LockStatus = VAL_ENUM(childval);
  }
  return res;
}
status_t build_to_priv_timesync_SyncSourceStatus(
    val_value_t *parentval,
    struct timesyncpb_SyncSourceStatus *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "LockStatus");
  if (childval != NULL && childval->res == NO_ERR) {
    /* enum */
    entry->LockStatus = VAL_ENUM(childval);
  }
  childval = val_first_child_name(
      parentval,
      "List");
  entry->List_Len = 0;
  entry->List = malloc(sizeof(*entry->List));
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
      res = build_to_priv_timesync_SyncSourceInputStatusEntry(
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
status_t build_to_priv_timesync_SyncSourceInputStatusEntry(
    val_value_t *parentval,
    struct timesyncpb_SyncSourceInputStatusEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "Source");
  if (childval != NULL && childval->res == NO_ERR) {
    /* enum */
    entry->Source = VAL_ENUM(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Signal");
  if (childval != NULL && childval->res == NO_ERR) {
    /* enum */
    entry->Signal = VAL_ENUM(childval);
  }
  return res;
}
