
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_acl_acl
#define _H_intri_pb_github_com_Intrising_intri_type_core_acl_acl

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/common/common.pb.h"
#include "../../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum aclpb_RuleActionTypeOptions {
  // [Unused] this rule is unused yet
  aclpb_RuleActionTypeOptions_RULE_ACTION_TYPE_UNUSED = 0,
  // [Permit] this rule action will permit packet
  aclpb_RuleActionTypeOptions_RULE_ACTION_TYPE_PERMIT = 1,
  // [Deny] this rule action will deny packet
  aclpb_RuleActionTypeOptions_RULE_ACTION_TYPE_DENY = 2,
  // *internal usage
  aclpb_RuleActionTypeOptions_RULE_ACTION_TYPE_MIRROR = 3,
};

enum aclpb_RuleParamTypeOptions {
  // [MAC] this rule param is MAC address specification
  aclpb_RuleParamTypeOptions_RULE_PARAM_TYPE_MAC = 0,
  // [IPv4] this rule param is IPv4 address specification
  aclpb_RuleParamTypeOptions_RULE_PARAM_TYPE_IP_V_4 = 1,
  // [IPv6] this rule param is IPv6 address specification
  aclpb_RuleParamTypeOptions_RULE_PARAM_TYPE_IP_V_6 = 2,
};

// *internal usage
enum aclpb_RuleDirectionTypeOptions {
  // *internal usage
  aclpb_RuleDirectionTypeOptions_RULE_DIRECTION_TYPE_INGRESS = 0,
  // *internal usage
  aclpb_RuleDirectionTypeOptions_RULE_DIRECTION_TYPE_EGRESS = 1,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct aclpb_Config {
  struct aclpb_InterfaceList *Interfaces;
  struct aclpb_ACLList *AclList;
  struct aclpb_ACEList *AceList;
  struct aclpb_BindingList *Binding;
  struct aclpb_FlowMirroringList *Flow;
};

struct aclpb_InterfaceList {
  unsigned int List_Len; // auto-gen: for list
  struct aclpb_InterfaceEntry **List;
};

struct aclpb_InterfaceEntry {
  // Index (for update); physical port
  struct devicepb_InterfaceIdentify *IdentifyNo;
  // should exist in `ACLList`
  char *IngressAclName;
  // should exist in `ACLList`
  char *EgressAclName;
};

struct aclpb_ACLList {
  // list range limit should in boundary (`BoundaryACL`, `name`)
  unsigned int List_Len; // auto-gen: for list
  struct aclpb_ACLEntry **List;
};

struct aclpb_ACLEntry {
  char *Name;
  // Index (for add/update/delete); unique
// should exist in `ACEList`
  unsigned int RuleList_Len; // auto-gen: for list
  char **RuleList;
};

struct aclpb_ACEList {
  // list range limit should in boundary (`BoundaryACL`, `rule`)
  unsigned int List_Len; // auto-gen: for list
  struct aclpb_ACEEntry **List;
};

struct aclpb_ACEEntry {
  // Index (for add/update/delete); unique
  char *Name;
  // string description = 2 [(validate.rules).string = {min_len: 0, max_len: 63636}]; // optionnal
  enum aclpb_RuleActionTypeOptions Action;
  long int Priority;
  // should exist in `TimeRangeConfig`
  char *TimeRangeName;
  enum aclpb_RuleParamTypeOptions ParamType;
  union {
    // filter vlan id in packet
// RuleVlan vlan = 7;
// filter mac address in packet
    struct aclpb_RuleMAC *ACEEntry_Param_Mac;
    // filter ipv4 in packet
    struct aclpb_RuleIPv4 *ACEEntry_Param_IPv4;
    // filter ipv6 in packet
    struct aclpb_RuleIPv6 *ACEEntry_Param_IPv6;
    // filter mac address and ipv4 in packet
    struct aclpb_RuleMACIPv4 *ACEEntry_Param_MacIPv4;
    // filter mac address and ipv6 in packet
    struct aclpb_RuleMACIPv6 *ACEEntry_Param_MacIPv6;
  };
};

struct aclpb_RuleVlan {
  // the field `vlan_i_d` should validated in vlan filter list
// https://github.com/Intrising/test-switch/issues/2615
  long int VlanID;
  // https://github.com/Intrising/test-switch/issues/2615
  long int VlanIDMask;
};

struct aclpb_RuleMACIPv4 {
  struct aclpb_RuleMAC *Mac;
  struct aclpb_RuleIPv4 *IPv4;
};

struct aclpb_RuleMACIPv6 {
  struct aclpb_RuleMAC *Mac;
  struct aclpb_RuleIPv6 *IPv6;
};

struct aclpb_RuleMAC {
  struct aclpb_EtherTypeConfig *EtherType;
  struct aclpb_MACConfig *Source;
  struct aclpb_MACConfig *Destination;
  long int VlanId;
};

struct aclpb_MACConfig {
  char *Address;
  char *AddressMask;
};

struct aclpb_EtherTypeConfig {
  char *Type;
  char *EtherTypeMask;
};

struct aclpb_IPProtocolConfig {
  char *Protocol;
  char *ProtocolMask;
};

struct aclpb_RuleIPv4 {
  struct aclpb_IPProtocolConfig *Protocol;
  struct aclpb_IPv4Config *Source;
  struct aclpb_IPv4Config *Destination;
  struct aclpb_RuleLayer4Port *Layer4Port;
};

struct aclpb_IPv4Config {
  char *Address;
  char *AddressMask;
};

struct aclpb_RuleIPv6 {
  struct aclpb_IPProtocolConfig *NextHeader;
  struct aclpb_IPv6Config *Source;
  struct aclpb_IPv6Config *Destination;
  struct aclpb_RuleLayer4Port *Layer4Port;
};

struct aclpb_IPv6Config {
  char *Address;
  char *AddressMask;
};

struct aclpb_RuleLayer4Port {
  struct aclpb_IPWithLayer4PortConfig *Source;
  struct aclpb_IPWithLayer4PortConfig *Destination;
};

struct aclpb_IPWithLayer4PortConfig {
  long int PortNumber;
  long int PortNumberMask;
};

struct aclpb_BindingList {
  // list range limit should in boundary (`BoundaryACL`, `binding`)
  unsigned int List_Len; // auto-gen: for list
  struct aclpb_BindingEntry **List;
};

struct aclpb_BindingEntry {
  // Index (for add/update/delete); unique
  char *Name;
  // physical port only
  struct devicepb_InterfaceIdentify *IdentifyNo;
  char *Mac;
  enum aclpb_RuleParamTypeOptions ParamType;
  union {
    char *BindingEntry_Param_IPv4;
    char *BindingEntry_Param_IPv6;
  };
};

struct aclpb_FlowMirroringEntry {
  // Index (for add/update/delete); unique
  char *Name;
  // the filed `a_c_l_name_list` should exist in `ACLList`, list range limit should in boundary (`BoundaryACL`, `flow_rules`)
  unsigned int ACLNameList_Len; // auto-gen: for list
  char **ACLNameList;
  // physical port only, the field `destination` interface should not in this list
  unsigned int SourceList_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **SourceList;
  // physical port only
  struct devicepb_InterfaceIdentify *Destination;
};

struct aclpb_FlowMirroringList {
  // list range limit should in boundary (`BoundaryACL`, `flow`)
  unsigned int List_Len; // auto-gen: for list
  struct aclpb_FlowMirroringEntry **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_acl_acl
