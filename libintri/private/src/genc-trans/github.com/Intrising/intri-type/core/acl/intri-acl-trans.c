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

#include "intri-acl-trans.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../github.com/Intrising/intri-type/common/intri-common-trans.h"
#include "../../../../../github.com/Intrising/intri-type/device/intri-device-trans.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"
status_t build_to_xml_acl_Config (
    val_value_t *parentval,
    struct aclpb_Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Interfaces",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_InterfaceList(
      childval,
    entry->Interfaces);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "AclList",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_ACLList(
      childval,
    entry->AclList);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "AceList",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_ACEList(
      childval,
    entry->AceList);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Binding",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_BindingList(
      childval,
    entry->Binding);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Flow",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_FlowMirroringList(
      childval,
    entry->Flow);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_acl_InterfaceList (
    val_value_t *parentval,
    struct aclpb_InterfaceList *entry) {
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
res =  build_to_xml_acl_InterfaceEntry(
    listval,
    entry->List[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  return res;
}

status_t build_to_xml_acl_InterfaceEntry (
    val_value_t *parentval,
    struct aclpb_InterfaceEntry *entry) {
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
    "IngressAclName",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->IngressAclName;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "EgressAclName",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->EgressAclName;
  return res;
}

status_t build_to_xml_acl_ACLList (
    val_value_t *parentval,
    struct aclpb_ACLList *entry) {
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
res =  build_to_xml_acl_ACLEntry(
    listval,
    entry->List[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  return res;
}

status_t build_to_xml_acl_ACLEntry (
    val_value_t *parentval,
    struct aclpb_ACLEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
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
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "RuleList",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* list */
  for (int i = 0; i < entry->RuleList_Len; i++) {
  val_value_t *listval = NULL;
listval =  agt_make_object(
    childval->obj,
    "RuleList_Entry",
    &res);
if (listval != NULL) {
  val_add_child(listval, childval);
} else if (res != NO_ERR) {
  return SET_ERROR(res);
}
VAL_STRING(listval) = entry->RuleList[i];
  }
  return res;
}

status_t build_to_xml_acl_ACEList (
    val_value_t *parentval,
    struct aclpb_ACEList *entry) {
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
res =  build_to_xml_acl_ACEEntry(
    listval,
    entry->List[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  return res;
}

status_t build_to_xml_acl_ACEEntry (
    val_value_t *parentval,
    struct aclpb_ACEEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
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
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Action",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->Action;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Priority",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->Priority;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "TimeRangeName",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->TimeRangeName;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "ParamType",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->ParamType;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Mac",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "IPv4",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "IPv6",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "MacIPv4",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "MacIPv6",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_acl_RuleVlan (
    val_value_t *parentval,
    struct aclpb_RuleVlan *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "VlanID",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->VlanID;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "VlanIDMask",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->VlanIDMask;
  return res;
}

status_t build_to_xml_acl_RuleMACIPv4 (
    val_value_t *parentval,
    struct aclpb_RuleMACIPv4 *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Mac",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_RuleMAC(
      childval,
    entry->Mac);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "IPv4",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_RuleIPv4(
      childval,
    entry->IPv4);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_acl_RuleMACIPv6 (
    val_value_t *parentval,
    struct aclpb_RuleMACIPv6 *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Mac",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_RuleMAC(
      childval,
    entry->Mac);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "IPv6",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_RuleIPv6(
      childval,
    entry->IPv6);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_acl_RuleMAC (
    val_value_t *parentval,
    struct aclpb_RuleMAC *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "EtherType",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_EtherTypeConfig(
      childval,
    entry->EtherType);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Source",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_MACConfig(
      childval,
    entry->Source);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Destination",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_MACConfig(
      childval,
    entry->Destination);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "VlanId",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->VlanId;
  return res;
}

status_t build_to_xml_acl_MACConfig (
    val_value_t *parentval,
    struct aclpb_MACConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Address",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->Address;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "AddressMask",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->AddressMask;
  return res;
}

status_t build_to_xml_acl_EtherTypeConfig (
    val_value_t *parentval,
    struct aclpb_EtherTypeConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Type",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->Type;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "EtherTypeMask",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->EtherTypeMask;
  return res;
}

status_t build_to_xml_acl_IPProtocolConfig (
    val_value_t *parentval,
    struct aclpb_IPProtocolConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Protocol",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->Protocol;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "ProtocolMask",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->ProtocolMask;
  return res;
}

status_t build_to_xml_acl_RuleIPv4 (
    val_value_t *parentval,
    struct aclpb_RuleIPv4 *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Protocol",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_IPProtocolConfig(
      childval,
    entry->Protocol);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Source",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_IPv4Config(
      childval,
    entry->Source);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Destination",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_IPv4Config(
      childval,
    entry->Destination);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Layer4Port",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_RuleLayer4Port(
      childval,
    entry->Layer4Port);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_acl_IPv4Config (
    val_value_t *parentval,
    struct aclpb_IPv4Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Address",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->Address;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "AddressMask",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->AddressMask;
  return res;
}

status_t build_to_xml_acl_RuleIPv6 (
    val_value_t *parentval,
    struct aclpb_RuleIPv6 *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "NextHeader",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_IPProtocolConfig(
      childval,
    entry->NextHeader);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Source",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_IPv6Config(
      childval,
    entry->Source);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Destination",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_IPv6Config(
      childval,
    entry->Destination);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Layer4Port",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_RuleLayer4Port(
      childval,
    entry->Layer4Port);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_acl_IPv6Config (
    val_value_t *parentval,
    struct aclpb_IPv6Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Address",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->Address;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "AddressMask",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->AddressMask;
  return res;
}

status_t build_to_xml_acl_RuleLayer4Port (
    val_value_t *parentval,
    struct aclpb_RuleLayer4Port *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Source",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_IPWithLayer4PortConfig(
      childval,
    entry->Source);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Destination",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_acl_IPWithLayer4PortConfig(
      childval,
    entry->Destination);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_acl_IPWithLayer4PortConfig (
    val_value_t *parentval,
    struct aclpb_IPWithLayer4PortConfig *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "PortNumber",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->PortNumber;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "PortNumberMask",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->PortNumberMask;
  return res;
}

status_t build_to_xml_acl_BindingList (
    val_value_t *parentval,
    struct aclpb_BindingList *entry) {
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
res =  build_to_xml_acl_BindingEntry(
    listval,
    entry->List[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  return res;
}

status_t build_to_xml_acl_BindingEntry (
    val_value_t *parentval,
    struct aclpb_BindingEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
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
    "Mac",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->Mac;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "ParamType",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  VAL_ENUM(childval) = entry->ParamType;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "IPv4",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "IPv6",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_acl_FlowMirroringEntry (
    val_value_t *parentval,
    struct aclpb_FlowMirroringEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
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
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "ACLNameList",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* list */
  for (int i = 0; i < entry->ACLNameList_Len; i++) {
  val_value_t *listval = NULL;
listval =  agt_make_object(
    childval->obj,
    "ACLNameList_Entry",
    &res);
if (listval != NULL) {
  val_add_child(listval, childval);
} else if (res != NO_ERR) {
  return SET_ERROR(res);
}
VAL_STRING(listval) = entry->ACLNameList[i];
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "SourceList",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* list */
  for (int i = 0; i < entry->SourceList_Len; i++) {
  val_value_t *listval = NULL;
listval =  agt_make_object(
    childval->obj,
    "SourceList_Entry",
    &res);
if (listval != NULL) {
  val_add_child(listval, childval);
} else if (res != NO_ERR) {
  return SET_ERROR(res);
}
res =  build_to_xml_device_InterfaceIdentify(
    listval,
    entry->SourceList[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "Destination",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
   build_to_xml_device_InterfaceIdentify(
      childval,
    entry->Destination);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}

status_t build_to_xml_acl_FlowMirroringList (
    val_value_t *parentval,
    struct aclpb_FlowMirroringList *entry) {
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
res =  build_to_xml_acl_FlowMirroringEntry(
    listval,
    entry->List[i]);
if (res != NO_ERR) {
  return SET_ERROR(res);
}
  }
  return res;
}
