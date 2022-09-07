
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_vlan_vlan
#define _H_intri_pb_github_com_Intrising_intri_type_core_vlan_vlan

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum vlanpb_StatusUsedTypeOptions {
  // [Config Usage] the VLAN is using by configuration
  vlanpb_StatusUsedTypeOptions_STATUS_USED_TYPE_CONFIG = 0,
  // [GVRP Usage] the VLAN is using by GVRP
  vlanpb_StatusUsedTypeOptions_STATUS_USED_TYPE_GVRP = 1,
  // [Via MAC Table]
  vlanpb_StatusUsedTypeOptions_STATUS_USED_TYPE_VIA_MAC_TABLE = 2,
  // [MAC Via Radius]
  vlanpb_StatusUsedTypeOptions_STATUS_USED_TYPE_MAC_VIA_RADIUS = 3,
  // [802.1x Via Radius]
  vlanpb_StatusUsedTypeOptions_STATUS_USED_TYPE_802_1X_VIA_RADIUS = 4,
  // [UnAuthorized]
  vlanpb_StatusUsedTypeOptions_STATUS_USED_TYPE_UNAUTHORIZED = 5,
};

enum vlanpb_PortConfigVLANModeTypeOptions {
  // [Access] the port interface is untagged port
  vlanpb_PortConfigVLANModeTypeOptions_PORT_CONFIG_VLAN_MODE_TYPE_ACCESS = 0,
  // [Hybrid] the port interface is tagged/untagged port
  vlanpb_PortConfigVLANModeTypeOptions_PORT_CONFIG_VLAN_MODE_TYPE_HYBRID = 1,
  // [Trunk] the port interface is tagged port
  vlanpb_PortConfigVLANModeTypeOptions_PORT_CONFIG_VLAN_MODE_TYPE_TRUNK = 2,
  // [Q-in-Q Customer] the port interface is tagged port, the overlapping VLAN IDs will be allow in Layer 2 Ethernet connection
  vlanpb_PortConfigVLANModeTypeOptions_PORT_CONFIG_VLAN_MODE_TYPE_QINQ_CUSTOMER = 3,
  // [Q-in-Q Provider] the port interface is tagged port, the overlapping VLAN IDs will be allow in Layer 2 Ethernet connection
  vlanpb_PortConfigVLANModeTypeOptions_PORT_CONFIG_VLAN_MODE_TYPE_QINQ_PROVIDER = 4,
};

enum vlanpb_PortConfigQinQEtherTypeOptions {
  // [0x8100] Normal VLAN tag usually not used for double tagged application. (801.1q)
  vlanpb_PortConfigQinQEtherTypeOptions_PORT_CONFIG_QINQ_ETHERTYPE_TYPE_0X_8100 = 0,
  // [0x88A8] Standard value for 802.1ad
  vlanpb_PortConfigQinQEtherTypeOptions_PORT_CONFIG_QINQ_ETHERTYPE_TYPE_0X_88A8 = 1,
  // [0x9100] Cisco standard value for 802.1ad
  vlanpb_PortConfigQinQEtherTypeOptions_PORT_CONFIG_QINQ_ETHERTYPE_TYPE_0X_9100 = 2,
};

enum vlanpb_ProtocolBasedEncapsulationTypeOptions {
  // [Ethernet V2]
  vlanpb_ProtocolBasedEncapsulationTypeOptions_PROTOCOL_BASED_ENCAPSULATION_TYPE_ETHERNET_V2 = 0,
  // [LLC]
  vlanpb_ProtocolBasedEncapsulationTypeOptions_PROTOCOL_BASED_ENCAPSULATION_TYPE_LLC = 1,
  // [LLC SNAP]
  vlanpb_ProtocolBasedEncapsulationTypeOptions_PROTOCOL_BASED_ENCAPSULATION_TYPE_LLCSNAP = 2,
};

enum vlanpb_IPVersionTypeOptions {
  // [IPv4]
  vlanpb_IPVersionTypeOptions_IP_VERSION_TYPE_V4 = 0,
  // [IPv6]
  vlanpb_IPVersionTypeOptions_IP_VERSION_TYPE_V6 = 1,
};

enum vlanpb_AcceptFrameTypeOptions {
  // [ALL]
  vlanpb_AcceptFrameTypeOptions_ACCEPT_FRAME_TYPE_ALL = 0,
  // [Untagged Only]
  vlanpb_AcceptFrameTypeOptions_ACCEPT_FRAME_TYPE_UNTAGGED_ONLY = 1,
  // [Tagged Only]
  vlanpb_AcceptFrameTypeOptions_ACCEPT_FRAME_TYPE_TAGGED_ONLY = 2,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

// Cannot be deleted if the GroupID is used by any GroupMember.
struct vlanpb_MACBasedGroupEntry {
  char *MACAddress;
  char *MACAddressMask;
  // Index (for update / delete); unique. boundary is according to device.GetBoundary().VLAN.MACBased
  int32_t GroupID;
};

struct vlanpb_MACBasedGroupMemberEntry {
  // Index (for update / delete); unique. Only  ID defined in Filters are available.
  int32_t VlanID;
  // GroupID CAN duplicate with other MemberEntry. boundary is according to device.GetBoundary().VLAN.MACBased
  int32_t GroupID;
  // Can be physical ports or LAG ports. Any interface cannot duplicated among other group member.
  unsigned int IdentifyList_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **IdentifyList;
};

struct vlanpb_MACBasedConfig {
  // boundary is according to device.GetBoundary().VLAN.MACBased
  unsigned int GroupList_Len; // auto-gen: for list
  struct vlanpb_MACBasedGroupEntry **GroupList;
  unsigned int GroupMemberList_Len; // auto-gen: for list
  struct vlanpb_MACBasedGroupMemberEntry **GroupMemberList;
};

// Cannot be deleted if the GroupID is used by any GroupMember.
struct vlanpb_ProtocolBasedGroupEntry {
  // Index (for update / delete); unique among groups. boundary is according to device.GetBoundary().VLAN.ProtocolBased
  int32_t GroupID;
  enum vlanpb_ProtocolBasedEncapsulationTypeOptions Encapsulation;
  // Hex in string, ex: `0x1234`. Range is 0x0600-0xffff
  char *Protocol;
};

struct vlanpb_ProtocolBasedGroupMemberEntry {
  // Index (for update / delete); unique among entries. Only  ID defined in Filters are available.
  int32_t VlanID;
  // GroupID CAN duplicate with other MemberEntry.  boundary is according to device.GetBoundary().VLAN.ProtocolBased
  int32_t GroupID;
  // Must be physical ports. Any interface cannot duplicated among other group member.
  unsigned int IdentifyList_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **IdentifyList;
};

struct vlanpb_ProtocolBasedConfig {
  // boundary is according to device.GetBoundary().VLAN.ProtocolBased
  unsigned int GroupList_Len; // auto-gen: for list
  struct vlanpb_ProtocolBasedGroupEntry **GroupList;
  unsigned int GroupMemberList_Len; // auto-gen: for list
  struct vlanpb_ProtocolBasedGroupMemberEntry **GroupMemberList;
};

struct vlanpb_SelectiveQinQConfig {
  // boundary is according to device.GetBoundary().VLAN.SelectiveQinQ
  unsigned int List_Len; // auto-gen: for list
  struct vlanpb_SelectiveQinQTranslatedEntry **List;
};

struct vlanpb_SelectiveQinQTranslatedEntry {
  // Index (for update / delete); unique among mappings. Only  ID defined in Filters are available.
  int32_t SourceVlanID;
  // Only  ID defined in Filters are available.
  int32_t TranslatedVlanID;
};

// Cannot be deleted if the GroupID is used by any GroupMember.
struct vlanpb_SubnetBasedGroupEntry {
  char *IPAddress;
  char *IPAddressMask;
  enum vlanpb_IPVersionTypeOptions IPVersion;
  // Index (for update / delete); unique  boundary is according to device.GetBoundary().VLAN.SubnetBased
  int32_t GroupID;
};

struct vlanpb_SubnetBasedGroupMemberEntry {
  // Index (for update / delete); unique. Only  ID defined in Filters are available.
  int32_t VlanID;
  // GroupID CAN duplicate with other MemberEntry.  boundary is according to device.GetBoundary().VLAN.SubnetBased
  int32_t GroupID;
  // Can be any type of ports. Any interface cannot duplicated among other group member.
  unsigned int IdentifyList_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **IdentifyList;
};

struct vlanpb_SubnetBasedConfig {
  // boundary is according to device.GetBoundary().VLAN.SubnetBased
  unsigned int GroupList_Len; // auto-gen: for list
  struct vlanpb_SubnetBasedGroupEntry **GroupList;
  unsigned int GroupMemberList_Len; // auto-gen: for list
  struct vlanpb_SubnetBasedGroupMemberEntry **GroupMemberList;
};

struct vlanpb_MappingPort {
  unsigned int List_Len; // auto-gen: for list
  struct vlanpb_MappingPortEntry **List;
};

struct vlanpb_MappingPortEntry {
  // when type is INTERFACE_TYPE_PORT, using device_i_d and port_no
// when type is INTERFACE_TYPE_TRUNK, using l_a_g_no
// other types are not supported.
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool Enabled;
};

struct vlanpb_MappingConfig {
  // boundary is according to device.GetBoundary().VLAN.Mapping
  unsigned int List_Len; // auto-gen: for list
  struct vlanpb_MappingEntry **List;
  unsigned int PortList_Len; // auto-gen: for list
  struct vlanpb_MappingPortEntry **PortList;
};

struct vlanpb_MappingEntry {
  // Index (for update / delete); unique among mappings. Only  ID defined in Filters are available.
  int32_t SourceVlanID;
  // Only  ID defined in Filters are available.
  int32_t TranslatedVlanID;
};

struct vlanpb_Config {
  struct vlanpb_ManagementConfig *Management;
  struct vlanpb_VoiceConfig *Voice;
  struct vlanpb_PortsConfig *Ports;
  struct vlanpb_FiltersConfig *Filters;
  struct vlanpb_MACBasedConfig *MACBased;
  struct vlanpb_SubnetBasedConfig *SubnetBased;
  struct vlanpb_ProtocolBasedConfig *ProtocolBased;
  struct vlanpb_MappingConfig *Translation;
  struct vlanpb_SelectiveQinQConfig *SelectiveQinQ;
};

struct vlanpb_ManagementConfig {
  // Only  ID defined in Filters are available.
  int32_t ManagementVlanID;
};

struct vlanpb_VoiceConfig {
  // Only  ID defined in Filters are available.
  int32_t VlanID;
  int32_t Prio;
  int32_t SignalPrio;
  int32_t DSCP;
  int32_t SignalDSCP;
};

struct vlanpb_PortsConfig {
  unsigned int List_Len; // auto-gen: for list
  struct vlanpb_PortEntry **List;
};

// 在 PortEntry 中，當 mode 為以下值時：
// - Access: 只能填 untagged list (裡面必須含有 default port vlan id)
// - Hybrid: 可填 tagged/untagged list (untagged list 必須含有該 port 的 default vlan id)
// - Trunk: 只能填 tagged list (裡面必須含有 default port vlan id)
// - QinQ Customer: 跟 Trunk 相同，QinQ ethertype 必須為 0x8100 (VLAN_PORT_CONFIG_QINQ_ETHERTYPE_0X_8100)
// - QinQ Provider: 跟 Trunk 相同，QinQ ethertype 可任意填
//
struct vlanpb_PortEntry {
  // when type is INTERFACE_TYPE_PORT, using device_i_d and port_no
// when type is INTERFACE_TYPE_TRUNK, using l_a_g_no
// other types are not supported.
  struct devicepb_InterfaceIdentify *IdentifyNo;
  enum vlanpb_PortConfigVLANModeTypeOptions Mode;
  // Only  ID defined in Filters are available.
  int32_t DefaultVlanID;
  // Only  ID defined in Filters are available.
  int32_t UnauthorizedVlanID;
  // Only  ID defined in Filters are available.
  int32_t FallBackVlanID;
  enum vlanpb_PortConfigQinQEtherTypeOptions QinQEthertype;
  enum vlanpb_AcceptFrameTypeOptions AcceptableFrametype;
  // Only  ID defined in Filters are available.
  unsigned int TaggedList_Len; // auto-gen: for list
  int32_t *TaggedList;
  // Only  ID defined in Filters are available.
  unsigned int UntaggedList_Len; // auto-gen: for list
  int32_t *UntaggedList;
};

struct vlanpb_FiltersConfig {
  // boundary is according to device.GetBoundary().VLAN.VlanFilter
  unsigned int List_Len; // auto-gen: for list
  struct vlanpb_FilterEntry **List;
};

// If any other setting (include VoiceConfig, ManagementConfig,
// MACBasedConfig, ProtocolBasedConfig, SubnetBasedConfig,
// MappingConfig) refer the  ID of the filter, this filter cannot be
// deleted.
struct vlanpb_FilterEntry {
  // boundary is according to device.GetBoundary().VLAN.VlanID, should be 1-4094
  int32_t VlanID;
  // MACBasedConfig, SubnetBasedConfig, ProtocolBasedConfig requires this enabled.
  bool Enabled;
  char *Name;
};

struct vlanpb_Used {
  enum vlanpb_StatusUsedTypeOptions Used;
};

struct vlanpb_VlanPortVlanEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  int32_t VlanID;
  enum vlanpb_StatusUsedTypeOptions Used;
};

// internal use
struct vlanpb_DefaultPortVlanEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  int32_t DefaultVlanID;
  enum vlanpb_StatusUsedTypeOptions LastUpdateMethod;
};

struct vlanpb_StatusEntry {
  int32_t VlanID;
  unsigned int TaggedList_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **TaggedList;
  unsigned int UntaggedList_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **UntaggedList;
};

struct vlanpb_StatusMapping_MappingEntry {
  int32_t Key;
  struct vlanpb_StatusEntry *Value;
};

struct vlanpb_StatusMapping {
  unsigned int Mapping_Len; // auto-gen: for list
  struct vlanpb_StatusMapping_MappingEntry **Mapping;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_vlan_vlan
