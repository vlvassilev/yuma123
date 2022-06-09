
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_multicast_multicast
#define _H_intri_pb_github_com_Intrising_intri_type_core_multicast_multicast

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

enum multicastpb_RouterDetectionTypeOptions {
  // [Query Message]
  multicastpb_RouterDetectionTypeOptions_ROUTER_DETECTION_TYPE_QUERY_MESSAGE = 0,
};

enum multicastpb_ProtocolTypeOptions {
  // [NONE]
  multicastpb_ProtocolTypeOptions_PROTOCOL_TYPE_NONE = 0,
  // [IGMP]
  multicastpb_ProtocolTypeOptions_PROTOCOL_TYPE_IGMP = 1,
  // [MLD]
  multicastpb_ProtocolTypeOptions_PROTOCOL_TYPE_MLD = 2,
  // [BOTH]
  multicastpb_ProtocolTypeOptions_PROTOCOL_TYPE_BOTH = 3,
};

enum multicastpb_RouterStatusTypeOptions {
  // [Disabled]
  multicastpb_RouterStatusTypeOptions_ROUTER_STATUS_TYPE_DISABLED = 0,
  // [Dynamic]
  multicastpb_RouterStatusTypeOptions_ROUTER_STATUS_TYPE_DYNAMIC = 1,
  // [Static]
  multicastpb_RouterStatusTypeOptions_ROUTER_STATUS_TYPE_STATIC = 2,
  // [Both]
  multicastpb_RouterStatusTypeOptions_ROUTER_STATUS_TYPE_BOTH = 3,
};

enum multicastpb_VersionTypeOptions {
  // [IGMP V1]
  multicastpb_VersionTypeOptions_VERSION_TYPE_IGMP_V1 = 0,
  // [IGMP V2]
  multicastpb_VersionTypeOptions_VERSION_TYPE_IGMP_V2 = 1,
  // [IGMP V3]
  multicastpb_VersionTypeOptions_VERSION_TYPE_IGMP_V3 = 2,
  // [MLD V1]
  multicastpb_VersionTypeOptions_VERSION_TYPE_MLD_V1 = 3,
  // [MLD V2]
  multicastpb_VersionTypeOptions_VERSION_TYPE_MLD_V2 = 4,
  // [Disabled]
  multicastpb_VersionTypeOptions_VERSION_TYPE_DISABLED = 5,
};

enum multicastpb_StaticTypeOptions {
  // [L2]
  multicastpb_StaticTypeOptions_STATIC_TYPE_L2 = 0,
  // [L3]
  multicastpb_StaticTypeOptions_STATIC_TYPE_L3 = 1,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct multicastpb_Config {
  bool IGMPSnoopingGlobalEnabled;
  bool MLDSnoopingGlobalEnabled;
  struct multicastpb_Snooping *IGMPConfig;
  struct multicastpb_Snooping *MLDConfig;
  struct multicastpb_RouterPort *RouterConfig;
  struct multicastpb_UnregisterFlood *UnregisterConfig;
  struct multicastpb_Static *StaticGroups;
};

struct multicastpb_Snooping {
  // boundary is according to device.GetBoundary().VLAN.VlanFilter
  unsigned int VlanList_Len; // auto-gen: for list
  struct multicastpb_SnoopingConfigEntry **VlanList;
};

struct multicastpb_SnoopingConfigEntry {
  // Index (only for update)
  long int VlanID;
  bool SnoopingEnabled;
  enum multicastpb_VersionTypeOptions QuerierVersion;
  enum multicastpb_RouterDetectionTypeOptions MulticastRouterDetection;
  bool EnableFastLeave;
  bool EnableReportSuppression;
  long int GroupLimit;
  // The unit of this field is second
  long int GroupMembershipInterval;
  // The unit of this field is second
  long int LastMemberQueryTime;
  // The unit of this field is second
  long int MaxResponseTime;
  // The unit of this field is second
  long int RouterAgingTime;
  // The unit of this field is second
  long int StartQueryCount;
  // The unit of this field is second
  long int StartQueryInterval;
  // The unit of this field is second
  long int QueryInterval;
  // The unit of this field is second
  long int Robustness;
  unsigned int IdentifyiesConfig_Len; // auto-gen: for list
  struct multicastpb_PortEnabledEntry **IdentifyiesConfig;
};

struct multicastpb_UnregisterFlood {
  // boundary is according to device.GetBoundary().VLAN.VlanFilter
  unsigned int VlanList_Len; // auto-gen: for list
  struct multicastpb_UnregisterFloodVlan **VlanList;
};

struct multicastpb_UnregisterFloodVlan {
  // Index (for update)
  long int VlanID;
  bool Enabled;
};

struct multicastpb_RouterPort {
  // boundary is according to device.GetBoundary().VLAN.VlanFilter
  unsigned int VlanList_Len; // auto-gen: for list
  struct multicastpb_RouterPortVlanEntry **VlanList;
};

struct multicastpb_RouterPortVlanEntry {
  // Index (for update)
  long int VlanID;
  unsigned int PortList_Len; // auto-gen: for list
  struct multicastpb_PortEnabledEntry **PortList;
};

struct multicastpb_PortEnabledEntry {
  // Index (for update); physical + lag port
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool Enabled;
};

struct multicastpb_Static {
  // boundary is according to device.GetBoundary().VLAN.VlanFilter
  unsigned int List_Len; // auto-gen: for list
  struct multicastpb_StaticGroupsConfigEntry **List;
};

struct multicastpb_StaticGroupsConfigEntry {
  // vlan_i_d+multicast_address (for add/update/delete); should exist in vlan filter list
  long int VlanID;
  char *Name;
  // ipv4 or ipv6 address
  char *MulticastAddress;
  // physical + lag port, should not duplicated in list
  struct devicepb_PortList *ForwardingPort;
};

struct multicastpb_IGMPStatistics {
  unsigned int List_Len; // auto-gen: for list
  struct multicastpb_IGMPStatisticEntry **List;
};

struct multicastpb_IGMPStatisticEntry {
  long int VlanID;
  // The unit of this field is packet
  long int RxGeneralQueries;
  // The unit of this field is packet
  long int RxV3Reports;
  // The unit of this field is packet
  long int RxV2Reports;
  // The unit of this field is packet
  long int RxV2Leaves;
  // The unit of this field is packet
  long int RxV1Reports;
  bool Querier;
};

struct multicastpb_RouterStatus {
  unsigned int VlanList_Len; // auto-gen: for list
  struct multicastpb_RouterStatusEntry **VlanList;
};

struct multicastpb_RouterStatusEntry {
  long int VlanID;
  unsigned int List_Len; // auto-gen: for list
  struct multicastpb_RouterPortStatusEntry **List;
};

struct multicastpb_RouterPortStatusEntry {
  enum multicastpb_ProtocolTypeOptions Type;
  enum multicastpb_RouterStatusTypeOptions Status;
  struct devicepb_InterfaceIdentify *IdentifyNo;
};

struct multicastpb_DynamicGroups {
  unsigned int List_Len; // auto-gen: for list
  struct multicastpb_DynamicGroupEntry **List;
};

struct multicastpb_DynamicGroupEntry {
  long int VlanID;
  char *Address;
  long int TTL;
  unsigned int IdentifyList_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **IdentifyList;
};

//MLD
struct multicastpb_MLDStatistics {
  unsigned int List_Len; // auto-gen: for list
  struct multicastpb_MLDStatisticEntry **List;
};

struct multicastpb_MLDStatisticEntry {
  long int VlanID;
  // The unit of this field is packet
  long int RxGeneralQueries;
  // The unit of this field is packet
  long int RxV2Reports;
  // The unit of this field is packet
  long int RxV1Reports;
  // The unit of this field is packet
  long int RxV1Leaves;
  bool Querier;
};

struct multicastpb_VlanList {
  unsigned int List_Len; // auto-gen: for list
  long int *List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_multicast_multicast
