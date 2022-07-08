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

#include "intri-cdp-trans.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

status_t build_to_xml_cdp_Config(
    val_value_t *parentval,
    struct cdppb_Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "SystemConfig",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_cdp_SystemConfig(
      childval,
      entry->SystemConfig);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}
status_t build_to_xml_cdp_SystemConfig(
    val_value_t *parentval,
    struct cdppb_SystemConfig *entry) {
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
      "TimeToLive",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->TimeToLive;
  childval = agt_make_object(
      parentval->obj,
      "MsgTxInterval",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->MsgTxInterval;
  childval = agt_make_object(
      parentval->obj,
      "CDPVersion",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  switch (entry->CDPVersion) {
    case cdppb_VersionTypeOptions_VERSION_TYPE_V1:
      enum_str = "VERSION_TYPE_V1";
      break;
    case cdppb_VersionTypeOptions_VERSION_TYPE_V2:
      enum_str = "VERSION_TYPE_V2";
      break;
  }
  VAL_ENUM_NAME(childval) = enum_str;
  return res;
}
status_t build_to_xml_cdp_LocalInfo(
    val_value_t *parentval,
    struct cdppb_LocalInfo *entry) {
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
    res = build_to_xml_cdp_LocalInfoEntry(
        listval,
        entry->List[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_xml_cdp_LocalInfoEntry(
    val_value_t *parentval,
    struct cdppb_LocalInfoEntry *entry) {
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
      "PowerAvailable",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_cdp_PowerAvailable(
      childval,
      entry->PowerAvailable);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "SystemInfo",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_cdp_SystemInfo(
      childval,
      entry->SystemInfo);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "VoiceVlan",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_cdp_VoiceVLAN(
      childval,
      entry->VoiceVlan);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "PortInfo",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_cdp_PortInfo(
      childval,
      entry->PortInfo);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "Capability",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  for (int i = 0; i < entry->Capability_Len; i++) {
    val_value_t *listval = NULL;
    listval = agt_make_object(
        childval->obj,
        "Capability_Entry",
        &res);
    if (listval != NULL) {
      val_add_child(listval, childval);
    } else if (res != NO_ERR) {
      return SET_ERROR(res);
    }
    /* enum */
    switch (entry->Capability[i]) {
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_ROUTER:
        enum_str = "CAPABILITY_TYPE_ROUTER";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_TRANSPARENT_BRIDGE:
        enum_str = "CAPABILITY_TYPE_TRANSPARENT_BRIDGE";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_SOURCE_ROUTE_BRIDGE:
        enum_str = "CAPABILITY_TYPE_SOURCE_ROUTE_BRIDGE";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_SWITCH:
        enum_str = "CAPABILITY_TYPE_SWITCH";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_HOST:
        enum_str = "CAPABILITY_TYPE_HOST";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_IGMP_CAPABLE:
        enum_str = "CAPABILITY_TYPE_IGMP_CAPABLE";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_REPEATER:
        enum_str = "CAPABILITY_TYPE_REPEATER";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_VOIP_PHONE:
        enum_str = "CAPABILITY_TYPE_VOIP_PHONE";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_REMOTELY_MANAGED_DEVICE:
        enum_str = "CAPABILITY_TYPE_REMOTELY_MANAGED_DEVICE";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_CVTA_STP_DISPUTE_RESOLUTION_CISCO_VT_CAMERA:
        enum_str = "CAPABILITY_TYPE_CVTA_STP_DISPUTE_RESOLUTION_CISCO_VT_CAMERA";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_TWO_PORT_MAC_REPLY:
        enum_str = "CAPABILITY_TYPE_TWO_PORT_MAC_REPLY";
        break;
    }
    VAL_ENUM_NAME(listval) = enum_str;
  }
  return res;
}
status_t build_to_xml_cdp_NeighborInfo(
    val_value_t *parentval,
    struct cdppb_NeighborInfo *entry) {
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
    res = build_to_xml_cdp_NeighborInfoEntry(
        listval,
        entry->List[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_xml_cdp_NeighborInfoEntry(
    val_value_t *parentval,
    struct cdppb_NeighborInfoEntry *entry) {
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
  res = build_to_xml_cdp_RecvIdentifyInfo(
      childval,
      entry->IdentifyNo);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "SystemInfo",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_cdp_SystemInfo(
      childval,
      entry->SystemInfo);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "PowerAvailable",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_cdp_PowerAvailable(
      childval,
      entry->PowerAvailable);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "TTL",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int64 */
  VAL_LONG(childval) = entry->TTL;
  childval = agt_make_object(
      parentval->obj,
      "VoIPVlan",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->VoIPVlan;
  return res;
}
status_t build_to_xml_cdp_RecvIdentifyInfo(
    val_value_t *parentval,
    struct cdppb_RecvIdentifyInfo *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "RecvIdentifyNo",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_device_InterfaceIdentify(
      childval,
      entry->RecvIdentifyNo);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "PortThroughInterface",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->PortThroughInterface;
  childval = agt_make_object(
      parentval->obj,
      "Capability",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  for (int i = 0; i < entry->Capability_Len; i++) {
    val_value_t *listval = NULL;
    listval = agt_make_object(
        childval->obj,
        "Capability_Entry",
        &res);
    if (listval != NULL) {
      val_add_child(listval, childval);
    } else if (res != NO_ERR) {
      return SET_ERROR(res);
    }
    /* enum */
    switch (entry->Capability[i]) {
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_ROUTER:
        enum_str = "CAPABILITY_TYPE_ROUTER";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_TRANSPARENT_BRIDGE:
        enum_str = "CAPABILITY_TYPE_TRANSPARENT_BRIDGE";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_SOURCE_ROUTE_BRIDGE:
        enum_str = "CAPABILITY_TYPE_SOURCE_ROUTE_BRIDGE";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_SWITCH:
        enum_str = "CAPABILITY_TYPE_SWITCH";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_HOST:
        enum_str = "CAPABILITY_TYPE_HOST";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_IGMP_CAPABLE:
        enum_str = "CAPABILITY_TYPE_IGMP_CAPABLE";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_REPEATER:
        enum_str = "CAPABILITY_TYPE_REPEATER";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_VOIP_PHONE:
        enum_str = "CAPABILITY_TYPE_VOIP_PHONE";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_REMOTELY_MANAGED_DEVICE:
        enum_str = "CAPABILITY_TYPE_REMOTELY_MANAGED_DEVICE";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_CVTA_STP_DISPUTE_RESOLUTION_CISCO_VT_CAMERA:
        enum_str = "CAPABILITY_TYPE_CVTA_STP_DISPUTE_RESOLUTION_CISCO_VT_CAMERA";
        break;
      case cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_TWO_PORT_MAC_REPLY:
        enum_str = "CAPABILITY_TYPE_TWO_PORT_MAC_REPLY";
        break;
    }
    VAL_ENUM_NAME(listval) = enum_str;
  }
  return res;
}
status_t build_to_xml_cdp_SystemInfo(
    val_value_t *parentval,
    struct cdppb_SystemInfo *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "IP",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  for (int i = 0; i < entry->IP_Len; i++) {
    val_value_t *listval = NULL;
    listval = agt_make_object(
        childval->obj,
        "IP_Entry",
        &res);
    if (listval != NULL) {
      val_add_child(listval, childval);
    } else if (res != NO_ERR) {
      return SET_ERROR(res);
    }
    /* string */
    VAL_STRING(listval) = entry->IP[i];
  }
  childval = agt_make_object(
      parentval->obj,
      "DeviceID",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->DeviceID;
  childval = agt_make_object(
      parentval->obj,
      "Platform",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->Platform;
  childval = agt_make_object(
      parentval->obj,
      "SystemName",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->SystemName;
  childval = agt_make_object(
      parentval->obj,
      "SoftwareVersion",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->SoftwareVersion;
  childval = agt_make_object(
      parentval->obj,
      "CDPVersion",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  switch (entry->CDPVersion) {
    case cdppb_VersionTypeOptions_VERSION_TYPE_V1:
      enum_str = "VERSION_TYPE_V1";
      break;
    case cdppb_VersionTypeOptions_VERSION_TYPE_V2:
      enum_str = "VERSION_TYPE_V2";
      break;
  }
  VAL_ENUM_NAME(childval) = enum_str;
  return res;
}
status_t build_to_xml_cdp_VoiceVLAN(
    val_value_t *parentval,
    struct cdppb_VoiceVLAN *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "Data",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bytes */
  VAL_STRING(childval) = entry->Data;
  childval = agt_make_object(
      parentval->obj,
      "VoiceVlan",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->VoiceVlan;
  return res;
}
status_t build_to_xml_cdp_PortInfo(
    val_value_t *parentval,
    struct cdppb_PortInfo *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "PortThroughInterface",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->PortThroughInterface;
  return res;
}
status_t build_to_xml_cdp_PowerAvailable(
    val_value_t *parentval,
    struct cdppb_PowerAvailable *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "RequestID",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->RequestID;
  childval = agt_make_object(
      parentval->obj,
      "ManagementID",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->ManagementID;
  childval = agt_make_object(
      parentval->obj,
      "Allocated",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* float */
  VAL_DOUBLE(childval) = entry->Allocated;
  childval = agt_make_object(
      parentval->obj,
      "Supported",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* float */
  VAL_DOUBLE(childval) = entry->Supported;
  return res;
}
status_t build_to_xml_cdp_NeighborPoe(
    val_value_t *parentval,
    struct cdppb_NeighborPoe *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "RequestId",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->RequestId;
  childval = agt_make_object(
      parentval->obj,
      "Allocated",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* float */
  VAL_DOUBLE(childval) = entry->Allocated;
  return res;
}
status_t build_to_xml_cdp_Statistic(
    val_value_t *parentval,
    struct cdppb_Statistic *entry) {
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
    res = build_to_xml_cdp_StatisticEntry(
        listval,
        entry->List[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_xml_cdp_StatisticEntry(
    val_value_t *parentval,
    struct cdppb_StatisticEntry *entry) {
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
      "FramesOutV1",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->FramesOutV1;
  childval = agt_make_object(
      parentval->obj,
      "FramesOutV2",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->FramesOutV2;
  childval = agt_make_object(
      parentval->obj,
      "FramesInV1",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->FramesInV1;
  childval = agt_make_object(
      parentval->obj,
      "FramesInV2",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->FramesInV2;
  childval = agt_make_object(
      parentval->obj,
      "IllegalChecksum",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->IllegalChecksum;
  childval = agt_make_object(
      parentval->obj,
      "OtherErrors",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->OtherErrors;
  return res;
}

status_t build_to_priv_cdp_Config(
    val_value_t *parentval,
    struct cdppb_Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "SystemConfig");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->SystemConfig = malloc(sizeof(*(entry->SystemConfig)));
    res = build_to_priv_cdp_SystemConfig(
        childval,
        entry->SystemConfig);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_priv_cdp_SystemConfig(
    val_value_t *parentval,
    struct cdppb_SystemConfig *entry) {
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
      "TimeToLive");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int32 */
    entry->TimeToLive = VAL_INT(childval);
  }
  childval = val_first_child_name(
      parentval,
      "MsgTxInterval");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int32 */
    entry->MsgTxInterval = VAL_INT(childval);
  }
  childval = val_first_child_name(
      parentval,
      "CDPVersion");
  if (childval != NULL && childval->res == NO_ERR) {
    /* enum */
    entry->CDPVersion = VAL_ENUM(childval);
  }
  return res;
}
status_t build_to_priv_cdp_LocalInfo(
    val_value_t *parentval,
    struct cdppb_LocalInfo *entry) {
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
      res = build_to_priv_cdp_LocalInfoEntry(
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
status_t build_to_priv_cdp_LocalInfoEntry(
    val_value_t *parentval,
    struct cdppb_LocalInfoEntry *entry) {
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
      "PowerAvailable");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->PowerAvailable = malloc(sizeof(*(entry->PowerAvailable)));
    res = build_to_priv_cdp_PowerAvailable(
        childval,
        entry->PowerAvailable);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "SystemInfo");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->SystemInfo = malloc(sizeof(*(entry->SystemInfo)));
    res = build_to_priv_cdp_SystemInfo(
        childval,
        entry->SystemInfo);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "VoiceVlan");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->VoiceVlan = malloc(sizeof(*(entry->VoiceVlan)));
    res = build_to_priv_cdp_VoiceVLAN(
        childval,
        entry->VoiceVlan);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "PortInfo");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->PortInfo = malloc(sizeof(*(entry->PortInfo)));
    res = build_to_priv_cdp_PortInfo(
        childval,
        entry->PortInfo);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "Capability");
  if (childval != NULL && childval->res == NO_ERR) {
    entry->Capability_Len = dlq_count(&childval->v.childQ);
    entry->Capability = malloc((entry->Capability_Len + 1) * sizeof(*entry->Capability));
    unsigned int cnt = 0;
    val_value_t *listval = NULL;
    for (listval = (val_value_t *)dlq_firstEntry(&childval->v.childQ);
         listval != NULL;
         listval = (val_value_t *)dlq_nextEntry(listval)) {
      /* enum */
      entry->Capability[cnt] = VAL_ENUM(listval);
      cnt++;
    }
  }
  return res;
}
status_t build_to_priv_cdp_NeighborInfo(
    val_value_t *parentval,
    struct cdppb_NeighborInfo *entry) {
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
      res = build_to_priv_cdp_NeighborInfoEntry(
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
status_t build_to_priv_cdp_NeighborInfoEntry(
    val_value_t *parentval,
    struct cdppb_NeighborInfoEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "IdentifyNo");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->IdentifyNo = malloc(sizeof(*(entry->IdentifyNo)));
    res = build_to_priv_cdp_RecvIdentifyInfo(
        childval,
        entry->IdentifyNo);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "SystemInfo");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->SystemInfo = malloc(sizeof(*(entry->SystemInfo)));
    res = build_to_priv_cdp_SystemInfo(
        childval,
        entry->SystemInfo);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "PowerAvailable");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->PowerAvailable = malloc(sizeof(*(entry->PowerAvailable)));
    res = build_to_priv_cdp_PowerAvailable(
        childval,
        entry->PowerAvailable);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "TTL");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int64 */
    entry->TTL = VAL_LONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "VoIPVlan");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int32 */
    entry->VoIPVlan = VAL_INT(childval);
  }
  return res;
}
status_t build_to_priv_cdp_RecvIdentifyInfo(
    val_value_t *parentval,
    struct cdppb_RecvIdentifyInfo *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "RecvIdentifyNo");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->RecvIdentifyNo = malloc(sizeof(*(entry->RecvIdentifyNo)));
    res = build_to_priv_device_InterfaceIdentify(
        childval,
        entry->RecvIdentifyNo);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "PortThroughInterface");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->PortThroughInterface = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Capability");
  if (childval != NULL && childval->res == NO_ERR) {
    entry->Capability_Len = dlq_count(&childval->v.childQ);
    entry->Capability = malloc((entry->Capability_Len + 1) * sizeof(*entry->Capability));
    unsigned int cnt = 0;
    val_value_t *listval = NULL;
    for (listval = (val_value_t *)dlq_firstEntry(&childval->v.childQ);
         listval != NULL;
         listval = (val_value_t *)dlq_nextEntry(listval)) {
      /* enum */
      entry->Capability[cnt] = VAL_ENUM(listval);
      cnt++;
    }
  }
  return res;
}
status_t build_to_priv_cdp_SystemInfo(
    val_value_t *parentval,
    struct cdppb_SystemInfo *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "IP");
  if (childval != NULL && childval->res == NO_ERR) {
    entry->IP_Len = dlq_count(&childval->v.childQ);
    entry->IP = malloc((entry->IP_Len + 1) * sizeof(*entry->IP));
    unsigned int cnt = 0;
    val_value_t *listval = NULL;
    for (listval = (val_value_t *)dlq_firstEntry(&childval->v.childQ);
         listval != NULL;
         listval = (val_value_t *)dlq_nextEntry(listval)) {
      /* string */
      entry->IP[cnt] = VAL_STRING(listval);
      cnt++;
    }
  }
  childval = val_first_child_name(
      parentval,
      "DeviceID");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->DeviceID = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Platform");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->Platform = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "SystemName");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->SystemName = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "SoftwareVersion");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->SoftwareVersion = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "CDPVersion");
  if (childval != NULL && childval->res == NO_ERR) {
    /* enum */
    entry->CDPVersion = VAL_ENUM(childval);
  }
  return res;
}
status_t build_to_priv_cdp_VoiceVLAN(
    val_value_t *parentval,
    struct cdppb_VoiceVLAN *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "Data");
  if (childval != NULL && childval->res == NO_ERR) {
    /* bytes */
    entry->Data = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "VoiceVlan");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int32 */
    entry->VoiceVlan = VAL_INT(childval);
  }
  return res;
}
status_t build_to_priv_cdp_PortInfo(
    val_value_t *parentval,
    struct cdppb_PortInfo *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "PortThroughInterface");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->PortThroughInterface = VAL_STRING(childval);
  }
  return res;
}
status_t build_to_priv_cdp_PowerAvailable(
    val_value_t *parentval,
    struct cdppb_PowerAvailable *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "RequestID");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int32 */
    entry->RequestID = VAL_INT(childval);
  }
  childval = val_first_child_name(
      parentval,
      "ManagementID");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int32 */
    entry->ManagementID = VAL_INT(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Allocated");
  if (childval != NULL && childval->res == NO_ERR) {
    /* float */
    entry->Allocated = VAL_DOUBLE(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Supported");
  if (childval != NULL && childval->res == NO_ERR) {
    /* float */
    entry->Supported = VAL_DOUBLE(childval);
  }
  return res;
}
status_t build_to_priv_cdp_NeighborPoe(
    val_value_t *parentval,
    struct cdppb_NeighborPoe *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "RequestId");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int32 */
    entry->RequestId = VAL_INT(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Allocated");
  if (childval != NULL && childval->res == NO_ERR) {
    /* float */
    entry->Allocated = VAL_DOUBLE(childval);
  }
  return res;
}
status_t build_to_priv_cdp_Statistic(
    val_value_t *parentval,
    struct cdppb_Statistic *entry) {
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
      res = build_to_priv_cdp_StatisticEntry(
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
status_t build_to_priv_cdp_StatisticEntry(
    val_value_t *parentval,
    struct cdppb_StatisticEntry *entry) {
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
      "FramesOutV1");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->FramesOutV1 = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "FramesOutV2");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->FramesOutV2 = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "FramesInV1");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->FramesInV1 = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "FramesInV2");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->FramesInV2 = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "IllegalChecksum");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->IllegalChecksum = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "OtherErrors");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->OtherErrors = VAL_ULONG(childval);
  }
  return res;
}
