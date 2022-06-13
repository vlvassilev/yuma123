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

#include "intri-lacp-trans.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"
status_t build_to_xml_lacp_Config (
    val_value_t *parentval,
    struct lacppb_Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "System",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_lacp_SystemConfig(
      childval,
    entry->System);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "LAG",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_lacp_LAGConfig(
      childval,
    entry->LAG);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_lacp_SystemConfig (
    val_value_t *parentval,
    struct lacppb_SystemConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "SystemPriority",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->SystemPriority;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Mode",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->Mode;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "TransmitInterval",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->TransmitInterval;
  return res;
}

status_t build_to_xml_lacp_LAGConfig (
    val_value_t *parentval,
    struct lacppb_LAGConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Lists",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* list */
  for (int i = 0; i < entry->Lists_Len; i++) {
  val_value_t *listval = NULL;
listval =  agt_make_object(
    childval->obj,
    "Lists_Entry",
    &res);
if (listval != NULL) {
  val_add_child(listval, childval);
} else if (res != NO_ERR) {
  return SET_ERROR(res);
}
res =  build_to_xml_lacp_LAGConfigEntry(
    listval,
    entry->Lists[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  return res;
}

status_t build_to_xml_lacp_LAGConfigEntry (
    val_value_t *parentval,
    struct lacppb_LAGConfigEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "TrunkID",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->TrunkID;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "LacpEnable",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->LacpEnable;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Identify",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_device_PortList(
      childval,
    entry->Identify);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_lacp_Status (
    val_value_t *parentval,
    struct lacppb_Status *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "PortLists",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* list */
  for (int i = 0; i < entry->PortLists_Len; i++) {
  val_value_t *listval = NULL;
listval =  agt_make_object(
    childval->obj,
    "PortLists_Entry",
    &res);
if (listval != NULL) {
  val_add_child(listval, childval);
} else if (res != NO_ERR) {
  return SET_ERROR(res);
}
res =  build_to_xml_lacp_StatusEntry(
    listval,
    entry->PortLists[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  return res;
}

status_t build_to_xml_lacp_StatusEntry (
    val_value_t *parentval,
    struct lacppb_StatusEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "TrunkID",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->TrunkID;
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
    "LacpEnable",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->LacpEnable;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Actor",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_lacp_ActorPartnerInfo(
      childval,
    entry->Actor);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Partner",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_lacp_ActorPartnerInfo(
      childval,
    entry->Partner);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_lacp_ActorPartnerInfo (
    val_value_t *parentval,
    struct lacppb_ActorPartnerInfo *entry) {
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
    "PortPriority",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->PortPriority;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "SystemPriority",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->SystemPriority;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "MACAddress",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bytes */
  VAL_STRING(childval) = entry->MACAddress;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "AdminKey",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->AdminKey;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "OperKey",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->OperKey;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Status",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_lacp_State(
      childval,
    entry->Status);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_lacp_State (
    val_value_t *parentval,
    struct lacppb_State *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Activity",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->Activity;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Timeout",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->Timeout;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Aggr",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->Aggr;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Sync",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->Sync;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Collect",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->Collect;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Distribute",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->Distribute;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Defaulted",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->Defaulted;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Expired",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->Expired;
  return res;
}
