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

#include "intri-dhcpserver-trans.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"

status_t build_to_xml_dhcpserver_Config(
    val_value_t *parentval,
    struct dhcpserverpb_Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "V4",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_dhcpserver_V4Config(
      childval,
      entry->V4);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}
status_t build_to_xml_dhcpserver_V4Config(
    val_value_t *parentval,
    struct dhcpserverpb_V4Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "Sys",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_dhcpserver_System(
      childval,
      entry->Sys);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "Pool",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_dhcpserver_Pool(
      childval,
      entry->Pool);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}
status_t build_to_xml_dhcpserver_System(
    val_value_t *parentval,
    struct dhcpserverpb_System *entry) {
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
status_t build_to_xml_dhcpserver_Pool(
    val_value_t *parentval,
    struct dhcpserverpb_Pool *entry) {
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
    res = build_to_xml_dhcpserver_PoolEntry(
        listval,
        entry->List[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_xml_dhcpserver_PoolEntry(
    val_value_t *parentval,
    struct dhcpserverpb_PoolEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "Name",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->Name;
  childval = agt_make_object(
      parentval->obj,
      "Interface",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_device_InterfaceIdentify(
      childval,
      entry->Interface);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "EntryType",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  switch (entry->EntryType) {
    case dhcpserverpb_PoolTypeOptions_POOL_TYPE_BASIC:
      enum_str = "POOL_TYPE_BASIC";
      break;
    case dhcpserverpb_PoolTypeOptions_POOL_TYPE_MAC_BASED:
      enum_str = "POOL_TYPE_MAC_BASED";
      break;
    case dhcpserverpb_PoolTypeOptions_POOL_TYPE_PORT_BASED:
      enum_str = "POOL_TYPE_PORT_BASED";
      break;
  }
  VAL_ENUM_NAME(childval) = enum_str;
  childval = agt_make_object(
      parentval->obj,
      "Basic",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_dhcpserver_Basic(
      childval,
      entry->Basic);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "MACBased",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_dhcpserver_MACBased(
      childval,
      entry->MACBased);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "PortBased",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_dhcpserver_PortBased(
      childval,
      entry->PortBased);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "ID",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->ID;
  return res;
}
status_t build_to_xml_dhcpserver_Basic(
    val_value_t *parentval,
    struct dhcpserverpb_Basic *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "RangeLow",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->RangeLow;
  childval = agt_make_object(
      parentval->obj,
      "RangeHigh",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->RangeHigh;
  childval = agt_make_object(
      parentval->obj,
      "Netmask",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->Netmask;
  childval = agt_make_object(
      parentval->obj,
      "Gateway",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->Gateway;
  childval = agt_make_object(
      parentval->obj,
      "PrimaryDNS",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->PrimaryDNS;
  childval = agt_make_object(
      parentval->obj,
      "SecondaryDNS",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->SecondaryDNS;
  childval = agt_make_object(
      parentval->obj,
      "LeaseTime",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->LeaseTime;
  return res;
}
status_t build_to_xml_dhcpserver_MACBased(
    val_value_t *parentval,
    struct dhcpserverpb_MACBased *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "MACAddress",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->MACAddress;
  childval = agt_make_object(
      parentval->obj,
      "DesiredIP",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->DesiredIP;
  return res;
}
status_t build_to_xml_dhcpserver_PortBased(
    val_value_t *parentval,
    struct dhcpserverpb_PortBased *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "PortNo",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->PortNo;
  childval = agt_make_object(
      parentval->obj,
      "Ignore",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->Ignore;
  childval = agt_make_object(
      parentval->obj,
      "DesiredIP",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->DesiredIP;
  return res;
}
status_t build_to_xml_dhcpserver_StatusEntry(
    val_value_t *parentval,
    struct dhcpserverpb_StatusEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "Name",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->Name;
  childval = agt_make_object(
      parentval->obj,
      "Interface",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_device_InterfaceIdentify(
      childval,
      entry->Interface);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "PortNo",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->PortNo;
  childval = agt_make_object(
      parentval->obj,
      "MACAddress",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->MACAddress;
  childval = agt_make_object(
      parentval->obj,
      "IPAddress",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->IPAddress;
  childval = agt_make_object(
      parentval->obj,
      "AvailableLeaseTime",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->AvailableLeaseTime;
  return res;
}
status_t build_to_xml_dhcpserver_Status(
    val_value_t *parentval,
    struct dhcpserverpb_Status *entry) {
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
    res = build_to_xml_dhcpserver_StatusEntry(
        listval,
        entry->List[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}

status_t build_to_priv_dhcpserver_Config(
    val_value_t *parentval,
    struct dhcpserverpb_Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "V4");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->V4 = malloc(sizeof(*(entry->V4)));
    res = build_to_priv_dhcpserver_V4Config(
        childval,
        entry->V4);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_priv_dhcpserver_V4Config(
    val_value_t *parentval,
    struct dhcpserverpb_V4Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "Sys");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->Sys = malloc(sizeof(*(entry->Sys)));
    res = build_to_priv_dhcpserver_System(
        childval,
        entry->Sys);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "Pool");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->Pool = malloc(sizeof(*(entry->Pool)));
    res = build_to_priv_dhcpserver_Pool(
        childval,
        entry->Pool);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_priv_dhcpserver_System(
    val_value_t *parentval,
    struct dhcpserverpb_System *entry) {
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
status_t build_to_priv_dhcpserver_Pool(
    val_value_t *parentval,
    struct dhcpserverpb_Pool *entry) {
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
      res = build_to_priv_dhcpserver_PoolEntry(
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
status_t build_to_priv_dhcpserver_PoolEntry(
    val_value_t *parentval,
    struct dhcpserverpb_PoolEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "Name");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->Name = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Interface");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->Interface = malloc(sizeof(*(entry->Interface)));
    res = build_to_priv_device_InterfaceIdentify(
        childval,
        entry->Interface);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "EntryType");
  if (childval != NULL && childval->res == NO_ERR) {
    /* enum */
    entry->EntryType = VAL_ENUM(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Basic");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->Basic = malloc(sizeof(*(entry->Basic)));
    res = build_to_priv_dhcpserver_Basic(
        childval,
        entry->Basic);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "MACBased");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->MACBased = malloc(sizeof(*(entry->MACBased)));
    res = build_to_priv_dhcpserver_MACBased(
        childval,
        entry->MACBased);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "PortBased");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->PortBased = malloc(sizeof(*(entry->PortBased)));
    res = build_to_priv_dhcpserver_PortBased(
        childval,
        entry->PortBased);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "ID");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->ID = VAL_STRING(childval);
  }
  return res;
}
status_t build_to_priv_dhcpserver_Basic(
    val_value_t *parentval,
    struct dhcpserverpb_Basic *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "RangeLow");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->RangeLow = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "RangeHigh");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->RangeHigh = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Netmask");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->Netmask = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Gateway");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->Gateway = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "PrimaryDNS");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->PrimaryDNS = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "SecondaryDNS");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->SecondaryDNS = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "LeaseTime");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int32 */
    entry->LeaseTime = VAL_INT(childval);
  }
  return res;
}
status_t build_to_priv_dhcpserver_MACBased(
    val_value_t *parentval,
    struct dhcpserverpb_MACBased *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "MACAddress");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->MACAddress = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "DesiredIP");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->DesiredIP = VAL_STRING(childval);
  }
  return res;
}
status_t build_to_priv_dhcpserver_PortBased(
    val_value_t *parentval,
    struct dhcpserverpb_PortBased *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "PortNo");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int32 */
    entry->PortNo = VAL_INT(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Ignore");
  if (childval != NULL && childval->res == NO_ERR) {
    /* bool */
    entry->Ignore = VAL_BOOL(childval);
  }
  childval = val_first_child_name(
      parentval,
      "DesiredIP");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->DesiredIP = VAL_STRING(childval);
  }
  return res;
}
status_t build_to_priv_dhcpserver_StatusEntry(
    val_value_t *parentval,
    struct dhcpserverpb_StatusEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "Name");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->Name = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Interface");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->Interface = malloc(sizeof(*(entry->Interface)));
    res = build_to_priv_device_InterfaceIdentify(
        childval,
        entry->Interface);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "PortNo");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int32 */
    entry->PortNo = VAL_INT(childval);
  }
  childval = val_first_child_name(
      parentval,
      "MACAddress");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->MACAddress = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "IPAddress");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->IPAddress = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "AvailableLeaseTime");
  if (childval != NULL && childval->res == NO_ERR) {
    /* int32 */
    entry->AvailableLeaseTime = VAL_INT(childval);
  }
  return res;
}
status_t build_to_priv_dhcpserver_Status(
    val_value_t *parentval,
    struct dhcpserverpb_Status *entry) {
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
      res = build_to_priv_dhcpserver_StatusEntry(
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
