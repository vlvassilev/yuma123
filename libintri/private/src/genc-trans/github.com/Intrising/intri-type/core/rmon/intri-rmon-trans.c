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

#include "intri-rmon-trans.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

status_t build_to_xml_rmon_IngressEntry(
    val_value_t *parentval,
    struct rmonpb_IngressEntry *entry) {
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
      "InGoodOctets",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->InGoodOctets;
  childval = agt_make_object(
      parentval->obj,
      "InBadOctets",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->InBadOctets;
  childval = agt_make_object(
      parentval->obj,
      "InTotalPackets",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->InTotalPackets;
  childval = agt_make_object(
      parentval->obj,
      "InUnicasts",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->InUnicasts;
  childval = agt_make_object(
      parentval->obj,
      "InNonUnicasts",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->InNonUnicasts;
  childval = agt_make_object(
      parentval->obj,
      "InBroadcasts",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->InBroadcasts;
  childval = agt_make_object(
      parentval->obj,
      "InMulticasts",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->InMulticasts;
  childval = agt_make_object(
      parentval->obj,
      "InPause",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->InPause;
  childval = agt_make_object(
      parentval->obj,
      "InTotalReceiveErrors",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->InTotalReceiveErrors;
  childval = agt_make_object(
      parentval->obj,
      "InUndersize",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->InUndersize;
  childval = agt_make_object(
      parentval->obj,
      "InOversize",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->InOversize;
  childval = agt_make_object(
      parentval->obj,
      "InFragments",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->InFragments;
  childval = agt_make_object(
      parentval->obj,
      "InJabber",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->InJabber;
  childval = agt_make_object(
      parentval->obj,
      "InFcsErrors",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->InFcsErrors;
  childval = agt_make_object(
      parentval->obj,
      "InDiscarded",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->InDiscarded;
  return res;
}
status_t build_to_xml_rmon_Ingress(
    val_value_t *parentval,
    struct rmonpb_Ingress *entry) {
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
    res = build_to_xml_rmon_IngressEntry(
        listval,
        entry->List[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_xml_rmon_EgressEntry(
    val_value_t *parentval,
    struct rmonpb_EgressEntry *entry) {
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
      "OutGoodOctets",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->OutGoodOctets;
  childval = agt_make_object(
      parentval->obj,
      "OutUnicasts",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->OutUnicasts;
  childval = agt_make_object(
      parentval->obj,
      "OutNonUnicasts",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->OutNonUnicasts;
  childval = agt_make_object(
      parentval->obj,
      "OutBroadcasts",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->OutBroadcasts;
  childval = agt_make_object(
      parentval->obj,
      "OutMulticasts",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->OutMulticasts;
  childval = agt_make_object(
      parentval->obj,
      "OutPause",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->OutPause;
  childval = agt_make_object(
      parentval->obj,
      "OutDeferred",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->OutDeferred;
  childval = agt_make_object(
      parentval->obj,
      "OutTotalCollisions",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->OutTotalCollisions;
  childval = agt_make_object(
      parentval->obj,
      "OutTotalPackets",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->OutTotalPackets;
  childval = agt_make_object(
      parentval->obj,
      "OutExcessiveCollisions",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->OutExcessiveCollisions;
  childval = agt_make_object(
      parentval->obj,
      "OutLateCollisions",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->OutLateCollisions;
  childval = agt_make_object(
      parentval->obj,
      "OutFcsErrors",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->OutFcsErrors;
  childval = agt_make_object(
      parentval->obj,
      "OutDroppedPackets",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->OutDroppedPackets;
  childval = agt_make_object(
      parentval->obj,
      "OutMultipleCollisions",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->OutMultipleCollisions;
  return res;
}
status_t build_to_xml_rmon_Egress(
    val_value_t *parentval,
    struct rmonpb_Egress *entry) {
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
    res = build_to_xml_rmon_EgressEntry(
        listval,
        entry->List[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_xml_rmon_HistogramEntry(
    val_value_t *parentval,
    struct rmonpb_HistogramEntry *entry) {
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
      "In_64Octets",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->In_64Octets;
  childval = agt_make_object(
      parentval->obj,
      "In_65To_127Octets",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->In_65To_127Octets;
  childval = agt_make_object(
      parentval->obj,
      "In_128To_255Octets",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->In_128To_255Octets;
  childval = agt_make_object(
      parentval->obj,
      "In_256To_511Octets",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->In_256To_511Octets;
  childval = agt_make_object(
      parentval->obj,
      "In_512To_1023Octets",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->In_512To_1023Octets;
  childval = agt_make_object(
      parentval->obj,
      "In_1024ToMaxOctets",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* uint64 */
  VAL_ULONG(childval) = entry->In_1024ToMaxOctets;
  return res;
}
status_t build_to_xml_rmon_Histogram(
    val_value_t *parentval,
    struct rmonpb_Histogram *entry) {
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
    res = build_to_xml_rmon_HistogramEntry(
        listval,
        entry->List[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_xml_rmon_UtilizationRate(
    val_value_t *parentval,
    struct rmonpb_UtilizationRate *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "Type",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  switch (entry->Type) {
    case rmonpb_UtilizationIntervalTypeOptions_UTILIZATION_INTERVAL_TYPE_NOW:
      enum_str = "UTILIZATION_INTERVAL_TYPE_NOW";
      break;
    case rmonpb_UtilizationIntervalTypeOptions_UTILIZATION_INTERVAL_TYPE_30_SECONDS:
      enum_str = "UTILIZATION_INTERVAL_TYPE_30_SECONDS";
      break;
    case rmonpb_UtilizationIntervalTypeOptions_UTILIZATION_INTERVAL_TYPE_5_MINUTES:
      enum_str = "UTILIZATION_INTERVAL_TYPE_5_MINUTES";
      break;
  }
  VAL_ENUM_NAME(childval) = enum_str;
  childval = agt_make_object(
      parentval->obj,
      "Rate",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->Rate;
  return res;
}
status_t build_to_xml_rmon_UtilizationEntry(
    val_value_t *parentval,
    struct rmonpb_UtilizationEntry *entry) {
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
      "Ingress",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  for (int i = 0; i < entry->Ingress_Len; i++) {
    val_value_t *listval = NULL;
    listval = agt_make_object(
        childval->obj,
        "Ingress_Entry",
        &res);
    if (listval != NULL) {
      val_add_child(listval, childval);
    } else if (res != NO_ERR) {
      return SET_ERROR(res);
    }
    /* message */
    res = build_to_xml_rmon_UtilizationRate(
        listval,
        entry->Ingress[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = agt_make_object(
      parentval->obj,
      "Egress",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  for (int i = 0; i < entry->Egress_Len; i++) {
    val_value_t *listval = NULL;
    listval = agt_make_object(
        childval->obj,
        "Egress_Entry",
        &res);
    if (listval != NULL) {
      val_add_child(listval, childval);
    } else if (res != NO_ERR) {
      return SET_ERROR(res);
    }
    /* message */
    res = build_to_xml_rmon_UtilizationRate(
        listval,
        entry->Egress[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_xml_rmon_Utilization(
    val_value_t *parentval,
    struct rmonpb_Utilization *entry) {
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
    res = build_to_xml_rmon_UtilizationEntry(
        listval,
        entry->List[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}

status_t build_to_priv_rmon_IngressEntry(
    val_value_t *parentval,
    struct rmonpb_IngressEntry *entry) {
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
      "InGoodOctets");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->InGoodOctets = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "InBadOctets");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->InBadOctets = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "InTotalPackets");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->InTotalPackets = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "InUnicasts");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->InUnicasts = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "InNonUnicasts");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->InNonUnicasts = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "InBroadcasts");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->InBroadcasts = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "InMulticasts");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->InMulticasts = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "InPause");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->InPause = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "InTotalReceiveErrors");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->InTotalReceiveErrors = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "InUndersize");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->InUndersize = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "InOversize");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->InOversize = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "InFragments");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->InFragments = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "InJabber");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->InJabber = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "InFcsErrors");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->InFcsErrors = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "InDiscarded");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->InDiscarded = VAL_ULONG(childval);
  }
  return res;
}
status_t build_to_priv_rmon_Ingress(
    val_value_t *parentval,
    struct rmonpb_Ingress *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
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
      res = build_to_priv_rmon_IngressEntry(
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
status_t build_to_priv_rmon_EgressEntry(
    val_value_t *parentval,
    struct rmonpb_EgressEntry *entry) {
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
      "OutGoodOctets");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->OutGoodOctets = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "OutUnicasts");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->OutUnicasts = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "OutNonUnicasts");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->OutNonUnicasts = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "OutBroadcasts");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->OutBroadcasts = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "OutMulticasts");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->OutMulticasts = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "OutPause");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->OutPause = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "OutDeferred");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->OutDeferred = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "OutTotalCollisions");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->OutTotalCollisions = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "OutTotalPackets");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->OutTotalPackets = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "OutExcessiveCollisions");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->OutExcessiveCollisions = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "OutLateCollisions");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->OutLateCollisions = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "OutFcsErrors");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->OutFcsErrors = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "OutDroppedPackets");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->OutDroppedPackets = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "OutMultipleCollisions");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->OutMultipleCollisions = VAL_ULONG(childval);
  }
  return res;
}
status_t build_to_priv_rmon_Egress(
    val_value_t *parentval,
    struct rmonpb_Egress *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
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
      res = build_to_priv_rmon_EgressEntry(
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
status_t build_to_priv_rmon_HistogramEntry(
    val_value_t *parentval,
    struct rmonpb_HistogramEntry *entry) {
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
      "In_64Octets");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->In_64Octets = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "In_65To_127Octets");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->In_65To_127Octets = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "In_128To_255Octets");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->In_128To_255Octets = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "In_256To_511Octets");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->In_256To_511Octets = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "In_512To_1023Octets");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->In_512To_1023Octets = VAL_ULONG(childval);
  }
  childval = val_first_child_name(
      parentval,
      "In_1024ToMaxOctets");
  if (childval != NULL && childval->res == NO_ERR) {
    /* uint64 */
    entry->In_1024ToMaxOctets = VAL_ULONG(childval);
  }
  return res;
}
status_t build_to_priv_rmon_Histogram(
    val_value_t *parentval,
    struct rmonpb_Histogram *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
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
      res = build_to_priv_rmon_HistogramEntry(
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
status_t build_to_priv_rmon_UtilizationRate(
    val_value_t *parentval,
    struct rmonpb_UtilizationRate *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "Type");
  if (childval != NULL && childval->res == NO_ERR) {
    /* enum */
    entry->Type = VAL_ENUM(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Rate");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int32 */
    entry->Rate = VAL_INT(childval);
  }
  return res;
}
status_t build_to_priv_rmon_UtilizationEntry(
    val_value_t *parentval,
    struct rmonpb_UtilizationEntry *entry) {
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
      "Ingress");
  entry->Ingress_Len = 0;
  entry->Ingress = malloc(sizeof(*entry->Ingress));
  if (childval != NULL && childval->res == NO_ERR) {
    entry->Ingress_Len = dlq_count(&childval->v.childQ);
    entry->Ingress = malloc((entry->Ingress_Len + 1) * sizeof(*entry->Ingress));
    unsigned int cnt = 0;
    val_value_t *listval = NULL;
    for (listval = (val_value_t *)dlq_firstEntry(&childval->v.childQ);
         listval != NULL;
         listval = (val_value_t *)dlq_nextEntry(listval)) {
      /* message */
      entry->Ingress[cnt] = malloc(sizeof(*(entry->Ingress[cnt])));
      res = build_to_priv_rmon_UtilizationRate(
          listval,
          entry->Ingress[cnt]);
      if (res != NO_ERR) {
        return SET_ERROR(res);
      }
      cnt++;
    }
  }
  childval = val_first_child_name(
      parentval,
      "Egress");
  entry->Egress_Len = 0;
  entry->Egress = malloc(sizeof(*entry->Egress));
  if (childval != NULL && childval->res == NO_ERR) {
    entry->Egress_Len = dlq_count(&childval->v.childQ);
    entry->Egress = malloc((entry->Egress_Len + 1) * sizeof(*entry->Egress));
    unsigned int cnt = 0;
    val_value_t *listval = NULL;
    for (listval = (val_value_t *)dlq_firstEntry(&childval->v.childQ);
         listval != NULL;
         listval = (val_value_t *)dlq_nextEntry(listval)) {
      /* message */
      entry->Egress[cnt] = malloc(sizeof(*(entry->Egress[cnt])));
      res = build_to_priv_rmon_UtilizationRate(
          listval,
          entry->Egress[cnt]);
      if (res != NO_ERR) {
        return SET_ERROR(res);
      }
      cnt++;
    }
  }
  return res;
}
status_t build_to_priv_rmon_Utilization(
    val_value_t *parentval,
    struct rmonpb_Utilization *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
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
      res = build_to_priv_rmon_UtilizationEntry(
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
