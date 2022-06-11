
#ifndef _H_intri_pb_github_com_Intrising_intri_type_cpss_cpss
#define _H_intri_pb_github_com_Intrising_intri_type_cpss_cpss

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>
#include <stdint.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum cpsspb_ActionTypeOptions {
  // [Add] Action type add
  cpsspb_ActionTypeOptions_ACTION_TYPE_ADD = 0,
  // [Delete] Action type delete
  cpsspb_ActionTypeOptions_ACTION_TYPE_DELETE = 1,
  // [Update] Action type update
  cpsspb_ActionTypeOptions_ACTION_TYPE_UPDATE = 2,
  // [Get] Action type get
  cpsspb_ActionTypeOptions_ACTION_TYPE_GET = 3,
};

enum cpsspb_InterfaceTypeOptions {
  // [Vlan] Interface type vlan
  cpsspb_InterfaceTypeOptions_INTERFACE_TYPE_VLAN = 0,
  // [Port] Interface type port
  cpsspb_InterfaceTypeOptions_INTERFACE_TYPE_PORT = 1,
  // [Trunk] Interface type trunk
  cpsspb_InterfaceTypeOptions_INTERFACE_TYPE_TRUNK = 2,
  // [Vidx] Interface type vidx
  cpsspb_InterfaceTypeOptions_INTERFACE_TYPE_VIDX = 3,
  // [Multicast] Interface type multicast
  cpsspb_InterfaceTypeOptions_INTERFACE_TYPE_MULTICAST = 4,
  // [Vlan Port] Interface type vlan port
  cpsspb_InterfaceTypeOptions_INTERFACE_TYPE_VLAN_PORT = 5,
  // [Vlan Trunk] Interface type vlan trunk
  cpsspb_InterfaceTypeOptions_INTERFACE_TYPE_VLAN_TRUNK = 6,
  // [Port Trunk] Interface type port trunk
  cpsspb_InterfaceTypeOptions_INTERFACE_TYPE_PORT_TRUNK = 7,
  // [Any] Interface type any
  cpsspb_InterfaceTypeOptions_INTERFACE_TYPE_ANY = 8,
};

enum cpsspb_PacketCommandOptions {
  cpsspb_PacketCommandOptions_PACKET_COMMAND_FORWARD = 0,
  cpsspb_PacketCommandOptions_PACKET_COMMAND_MIRROR = 1,
  cpsspb_PacketCommandOptions_PACKET_COMMAND_TRAP = 2,
  cpsspb_PacketCommandOptions_PACKET_COMMAND_DROP = 3,
};

enum cpsspb_PortSpeedDuplexOptions {
  cpsspb_PortSpeedDuplexOptions_PORT_SPEED_DUPLEX_AUTO = 0,
  cpsspb_PortSpeedDuplexOptions_PORT_SPEED_DUPLEX_10M_FULL = 1,
  cpsspb_PortSpeedDuplexOptions_PORT_SPEED_DUPLEX_100M_FULL = 3,
  cpsspb_PortSpeedDuplexOptions_PORT_SPEED_DUPLEX_1000M_FULL = 5,
  cpsspb_PortSpeedDuplexOptions_PORT_SPEED_DUPLEX_2500M_FULL = 6,
  cpsspb_PortSpeedDuplexOptions_PORT_SPEED_DUPLEX_5G_FULL = 7,
  cpsspb_PortSpeedDuplexOptions_PORT_SPEED_DUPLEX_10G_FULL = 8,
  cpsspb_PortSpeedDuplexOptions_PORT_SPEED_DUPLEX_25G_FULL = 9,
  cpsspb_PortSpeedDuplexOptions_PORT_SPEED_DUPLEX_40G_FULL = 10,
  cpsspb_PortSpeedDuplexOptions_PORT_SPEED_DUPLEX_100G_FULL = 11,
};

enum cpsspb_PortTypeOptions {
  // [10 M Full] Port type 10 m full
  cpsspb_PortTypeOptions_PORT_TYPE_10M_FULL = 0,
  // [100 M Full] Port type 100 m full
  cpsspb_PortTypeOptions_PORT_TYPE_100M_FULL = 2,
  // [1000 M Full] Port type 1000 m full
  cpsspb_PortTypeOptions_PORT_TYPE_1000M_FULL = 4,
  // [2500 M Full] Port type 2500 m full
  cpsspb_PortTypeOptions_PORT_TYPE_2500M_FULL = 5,
  // [5 G Full] Port type 5 g full
  cpsspb_PortTypeOptions_PORT_TYPE_5G_FULL = 6,
  // [10 G Full] Port type 10 g full
  cpsspb_PortTypeOptions_PORT_TYPE_10G_FULL = 7,
  // [25 G Full] Port type 25 g full
  cpsspb_PortTypeOptions_PORT_TYPE_25G_FULL = 8,
  // [40 G Full] Port type 40 g full
  cpsspb_PortTypeOptions_PORT_TYPE_40G_FULL = 9,
  // [100 G Full] Port type 100 g full
  cpsspb_PortTypeOptions_PORT_TYPE_100G_FULL = 10,
};

enum cpsspb_PortMediaOptions {
  cpsspb_PortMediaOptions_PORT_MEDIA_COPPER = 0,
  cpsspb_PortMediaOptions_PORT_MEDIA_RJ45 = 1,
  cpsspb_PortMediaOptions_PORT_MEDIA_SFP = 2,
  cpsspb_PortMediaOptions_PORT_MEDIA_STACKING = 3,
  cpsspb_PortMediaOptions_PORT_MEDIA_POE = 4,
  cpsspb_PortMediaOptions_PORT_MEDIA_POE_PLUS = 5,
  cpsspb_PortMediaOptions_PORT_MEDIA_POE_PLUS_PLUS = 6,
  cpsspb_PortMediaOptions_PORT_MEDIA_UP_LINK_PORT = 7,
  cpsspb_PortMediaOptions_PORT_MEDIA_DOWN_LINK_PORT = 8,
};

enum cpsspb_STPProtocolTypeOptions {
  // [Cpss] Stp protocol type cpss
  cpsspb_STPProtocolTypeOptions_STP_PROTOCOL_TYPE_CPSS = 0,
  // [Dhcp] Stp protocol type dhcp
  cpsspb_STPProtocolTypeOptions_STP_PROTOCOL_TYPE_DHCP = 10,
  // [Dhcp] Stp protocol type storm control
  cpsspb_STPProtocolTypeOptions_STP_PROTOCOL_TYPE_STORM_CONTROL = 20,
  // [Loop] Stp protocol type loop
  cpsspb_STPProtocolTypeOptions_STP_PROTOCOL_TYPE_LOOP = 30,
  // [Core] Stp protocol type port
  cpsspb_STPProtocolTypeOptions_STP_PROTOCOL_TYPE_PORT = 40,
  // [Lacp] Stp protocol type lacp
  cpsspb_STPProtocolTypeOptions_STP_PROTOCOL_TYPE_LACP = 70,
  // [Stp] Stp protocol type PACC
  cpsspb_STPProtocolTypeOptions_STP_PROTOCOL_TYPE_PACC = 80,
  // [Stp] Stp protocol type stp
  cpsspb_STPProtocolTypeOptions_STP_PROTOCOL_TYPE_STP = 90,
  // [G8032] Stp protocol type g8032
  cpsspb_STPProtocolTypeOptions_STP_PROTOCOL_TYPE_G8032 = 100,
};

enum cpsspb_STPPortStateTypeOptions {
  // [Disable] Stp port state type disable
  cpsspb_STPPortStateTypeOptions_STP_PORT_STATE_TYPE_DISABLE = 0,
  // [Blocking] Stp port state type blocking
  cpsspb_STPPortStateTypeOptions_STP_PORT_STATE_TYPE_BLOCKING = 1,
  // [Learning] Stp port state type learning
  cpsspb_STPPortStateTypeOptions_STP_PORT_STATE_TYPE_LEARNING = 2,
  // [Forwarding] Stp port state type forwarding
  cpsspb_STPPortStateTypeOptions_STP_PORT_STATE_TYPE_FORWARDING = 3,
};

enum cpsspb_ACLDirectionTypeOptions {
  // [Ingress] Acl direction type ingress
  cpsspb_ACLDirectionTypeOptions_ACL_DIRECTION_TYPE_INGRESS = 0,
  // [Egress] Acl direction type egress
  cpsspb_ACLDirectionTypeOptions_ACL_DIRECTION_TYPE_EGRESS = 1,
};

enum cpsspb_ACLRuleTypeOptions {
  // [Mac] Acl rule type mac
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_MAC = 0,
  // [Ipv4] Acl rule type ipv4
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_IPV4 = 1,
  // [Ipv6] Acl rule type ipv6
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_IPV6 = 2,
  // [Stp] Acl rule type stp
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_STP = 3,
  // [Lacp] Acl rule type lacp
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_LACP = 4,
  // [Dot1 X] Acl rule type dot1 x
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_DOT1X = 5,
  // [Arp] Acl rule type arp
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_ARP = 6,
  // [Dhcpv4 Src 67 Dst 68] Acl rule type dhcpv4 src 67 dst 68
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_DHCPV4_SRC_67_DST_68 = 7,
  // [Dhcpv4 Src 68 Dst 67] Acl rule type dhcpv4 src 68 dst 67
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_DHCPV4_SRC_68_DST_67 = 8,
  // [Loop] Acl rule type loop
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_LOOP = 9,
  // [G8032] Acl rule type g8032
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_G8032 = 10,
  // [Mac Ip Binding] Acl rule type mac ip binding
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_MAC_IP_BINDING = 11,
  // [Subnet Based] Acl rule type subnet based
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_SUBNET_BASED = 12,
  // [Mac Based] Acl rule type mac based
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_MAC_BASED = 13,
  // [Selective Qinq] Acl rule type selective qinq
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_SELECTIVE_QINQ = 14,
  // [Flow Mirror] Acl rule type flow mirror
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_FLOW_MIRROR = 15,
  // [Mac Counting] Acl rule type mac counting
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_MAC_COUNTING = 16,
  // [PTP Message] Acl rule type ptp message
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_PTP_MESSAGE = 17,
  // [Icoming Deny] Acl rule for deny the incoming packets
  cpsspb_ACLRuleTypeOptions_ACL_RULE_TYPE_INCOMING_BLOCK = 18,
};

enum cpsspb_CNCCountingTypeOptions {
  // [Mac Counting] Cnc counting type mac counting
  cpsspb_CNCCountingTypeOptions_CNC_COUNTING_TYPE_MAC_COUNTING = 0,
};

enum cpsspb_FDBLayerTypeOptions {
  // [Layer 2] FDB layer type 2
  cpsspb_FDBLayerTypeOptions_FDB_LAYER_TYPE_2 = 0,
  // [Layer 3] FDB layer type 3
  cpsspb_FDBLayerTypeOptions_FDB_LAYER_TYPE_3 = 1,
};

enum cpsspb_FDBEntryActionTypeOptions {
  // [Add] FDB entry action type add
  cpsspb_FDBEntryActionTypeOptions_FDB_ENTRY_ACTION_TYPE_ADD = 0,
  // [Delete] FDB entry action type delete
  cpsspb_FDBEntryActionTypeOptions_FDB_ENTRY_ACTION_TYPE_DELETE = 1,
};

enum cpsspb_MulticastEntryTypeOptions {
  // [MAC] Multicast entry type mac
  cpsspb_MulticastEntryTypeOptions_MULTICAST_ENTRY_TYPE_MAC = 0,
  // [IPv4] Multicast entry type i pv4
  cpsspb_MulticastEntryTypeOptions_MULTICAST_ENTRY_TYPE_IPV4 = 1,
  // [IPv6] Multicast entry type i pv6
  cpsspb_MulticastEntryTypeOptions_MULTICAST_ENTRY_TYPE_IPV6 = 2,
};

enum cpsspb_VIDXEntryTypeOptions {
  cpsspb_VIDXEntryTypeOptions_VIDX_ENTRY_TYPE_VLAN_FLOODING = 0,
  cpsspb_VIDXEntryTypeOptions_VIDX_ENTRY_TYPE_BRIDGE_MULTICAST = 1,
};

enum cpsspb_MirroringDirectionTypeOptions {
  // [None] Mirroring direction type none
  cpsspb_MirroringDirectionTypeOptions_MIRRORING_DIRECTION_TYPE_NONE = 0,
  // [Tx Only] Mirroring direction type tx only
  cpsspb_MirroringDirectionTypeOptions_MIRRORING_DIRECTION_TYPE_TX_ONLY = 1,
  // [Rx Only] Mirroring direction type rx only
  cpsspb_MirroringDirectionTypeOptions_MIRRORING_DIRECTION_TYPE_RX_ONLY = 2,
  // [Both] Mirroring direction type both
  cpsspb_MirroringDirectionTypeOptions_MIRRORING_DIRECTION_TYPE_BOTH = 3,
};

enum cpsspb_QoSTrustModeOptions {
  // [Disabled] Qos trust mode disabled
  cpsspb_QoSTrustModeOptions_QOS_TRUST_MODE_DISABLED = 0,
  // [CoS] Qos trust mode cos
  cpsspb_QoSTrustModeOptions_QOS_TRUST_MODE_COS = 1,
  // [DSCP Only] Qos trust mode dscp only
  cpsspb_QoSTrustModeOptions_QOS_TRUST_MODE_DSCP_ONLY = 2,
  // [DSCP First] Qos trust mode dscp first
  cpsspb_QoSTrustModeOptions_QOS_TRUST_MODE_DSCP_FIRST = 3,
};

enum cpsspb_QoSSchedulerTypeOptions {
  // [Weighted] Qos scheduler type weighted
  cpsspb_QoSSchedulerTypeOptions_QOS_SCHEDULER_TYPE_WEIGHTED = 0,
  // [Strict] Qos scheduler type strict
  cpsspb_QoSSchedulerTypeOptions_QOS_SCHEDULER_TYPE_STRICT = 1,
};

enum cpsspb_RateLimitingIngressFilterTypeOptions {
  // [UC Known] Rate limiting ingress filter type uc known
  cpsspb_RateLimitingIngressFilterTypeOptions_RATE_LIMITING_INGRESS_FILTER_TYPE_UC_KNOWN = 0,
  // [UC Unknown] Rate limiting ingress filter type uc unknown
  cpsspb_RateLimitingIngressFilterTypeOptions_RATE_LIMITING_INGRESS_FILTER_TYPE_UC_UNKNOWN = 1,
  // [MC Unregistered] Rate limiting ingress filter type mc unregistered
  cpsspb_RateLimitingIngressFilterTypeOptions_RATE_LIMITING_INGRESS_FILTER_TYPE_MC_UNREGISTERED = 2,
  // [MC Registered] Rate limiting ingress filter type mc registered
  cpsspb_RateLimitingIngressFilterTypeOptions_RATE_LIMITING_INGRESS_FILTER_TYPE_MC_REGISTERED = 3,
  // [BC] Rate limiting ingress filter type bc
  cpsspb_RateLimitingIngressFilterTypeOptions_RATE_LIMITING_INGRESS_FILTER_TYPE_BC = 4,
};

enum cpsspb_TrunkingLoadBalanceModeOptions {
  // [Source/Destination MAC] Trunking load balance mode src dst mac
  cpsspb_TrunkingLoadBalanceModeOptions_TRUNKING_LOAD_BALANCE_MODE_SRC_DST_MAC = 0,
  // [Source/Destination IP] Trunking load balance mode src dst ip
  cpsspb_TrunkingLoadBalanceModeOptions_TRUNKING_LOAD_BALANCE_MODE_SRC_DST_IP = 1,
  // [Source/Destination MAC and IP] Trunking load balance mode src dst mac ip
  cpsspb_TrunkingLoadBalanceModeOptions_TRUNKING_LOAD_BALANCE_MODE_SRC_DST_MAC_IP = 2,
};

enum cpsspb_VlanTaggingCommandTypeOptions {
  // [Untagged] Vlan tagging command type untagged
  cpsspb_VlanTaggingCommandTypeOptions_VLAN_TAGGING_COMMAND_TYPE_UNTAGGED = 0,
  // [Tagged] Vlan tagging command type tagged
  cpsspb_VlanTaggingCommandTypeOptions_VLAN_TAGGING_COMMAND_TYPE_TAGGED = 1,
  // [QinQ Provider] Vlan tagging command type qinq provider
  cpsspb_VlanTaggingCommandTypeOptions_VLAN_TAGGING_COMMAND_TYPE_QINQ_PROVIDER = 2,
  // [QinQ Customer] Vlan tagging command type qinq customer
  cpsspb_VlanTaggingCommandTypeOptions_VLAN_TAGGING_COMMAND_TYPE_QINQ_CUSTOMER = 3,
};

enum cpsspb_VlanAcceptFrameTypeOptions {
  // [All] Vlan accept frame type all
  cpsspb_VlanAcceptFrameTypeOptions_VLAN_ACCEPT_FRAME_TYPE_ALL = 0,
  // [Untagged Only] Vlan accept frame type untagged only
  cpsspb_VlanAcceptFrameTypeOptions_VLAN_ACCEPT_FRAME_TYPE_UNTAGGED_ONLY = 1,
  // [Tagged Only] Vlan accept frame type tagged only
  cpsspb_VlanAcceptFrameTypeOptions_VLAN_ACCEPT_FRAME_TYPE_TAGGED_ONLY = 2,
};

enum cpsspb_VlanTranslationMappingTypeOptions {
  // [One To One] Vlan translation mapping type one to one
  cpsspb_VlanTranslationMappingTypeOptions_VLAN_TRANSLATION_MAPPING_TYPE_ONE_TO_ONE = 0,
  // [Many To One] Vlan translation mapping type many to one
  cpsspb_VlanTranslationMappingTypeOptions_VLAN_TRANSLATION_MAPPING_TYPE_MANY_TO_ONE = 1,
};

enum cpsspb_VlanProtocolBasedEncapsulationTypeOptions {
  // [Ethernet V2] Vlan protocol based encapsulation ethernet v2
  cpsspb_VlanProtocolBasedEncapsulationTypeOptions_VLAN_PROTOCOL_BASED_ENCAPSULATION_ETHERNET_V2 = 0,
  // [None LLC Snap] Vlan protocol based encapsulation non llc snap
  cpsspb_VlanProtocolBasedEncapsulationTypeOptions_VLAN_PROTOCOL_BASED_ENCAPSULATION_NON_LLC_SNAP = 1,
  // [LLC Snap] Vlan protocol based encapsulation llc snap
  cpsspb_VlanProtocolBasedEncapsulationTypeOptions_VLAN_PROTOCOL_BASED_ENCAPSULATION_LLC_SNAP = 2,
};

enum cpsspb_PacketControlProtocolTypeOptions {
  // [STP] Packet control protocol type stp
  cpsspb_PacketControlProtocolTypeOptions_PACKET_CONTROL_PROTOCOL_TYPE_STP = 0,
  // [LACP] Packet control protocol type lacp
  cpsspb_PacketControlProtocolTypeOptions_PACKET_CONTROL_PROTOCOL_TYPE_LACP = 1,
  // [Dot1X] Packet control protocol type dot1 x
  cpsspb_PacketControlProtocolTypeOptions_PACKET_CONTROL_PROTOCOL_TYPE_DOT1X = 2,
  // [Loop] Packet control protocol type loop
  cpsspb_PacketControlProtocolTypeOptions_PACKET_CONTROL_PROTOCOL_TYPE_LOOP = 3,
  // [DLDP] Packet control protocol type dldp
  cpsspb_PacketControlProtocolTypeOptions_PACKET_CONTROL_PROTOCOL_TYPE_DLDP = 4,
  // [DHCP V4 Discover] Packet control protocol type dhcpv4 discover
  cpsspb_PacketControlProtocolTypeOptions_PACKET_CONTROL_PROTOCOL_TYPE_DHCPV4_DISCOVER = 5,
  // [DHCP V4 Broadcast Offer] Packet control protocol type dhcpv4 broadcast offer
  cpsspb_PacketControlProtocolTypeOptions_PACKET_CONTROL_PROTOCOL_TYPE_DHCPV4_BROADCAST_OFFER = 6,
  // [DHCP V4 UDP Destination 67] Packet control protocol type dhcpv4 udp dst 67
  cpsspb_PacketControlProtocolTypeOptions_PACKET_CONTROL_PROTOCOL_TYPE_DHCPV4_UDP_DST_67 = 7,
  // [DHCP V4 UDP Destination 68] Packet control protocol type dhcpv4 udp dst 68
  cpsspb_PacketControlProtocolTypeOptions_PACKET_CONTROL_PROTOCOL_TYPE_DHCPV4_UDP_DST_68 = 8,
  // [ARP] Packet control protocol type arp
  cpsspb_PacketControlProtocolTypeOptions_PACKET_CONTROL_PROTOCOL_TYPE_ARP = 9,
  // [GVRP] Packet control protocol type gvrp
  cpsspb_PacketControlProtocolTypeOptions_PACKET_CONTROL_PROTOCOL_TYPE_GVRP = 10,
  // [IGMP] Packet control protocol type igmp
  cpsspb_PacketControlProtocolTypeOptions_PACKET_CONTROL_PROTOCOL_TYPE_IGMP = 11,
  // [MLD] Packet control protocol type mld
  cpsspb_PacketControlProtocolTypeOptions_PACKET_CONTROL_PROTOCOL_TYPE_MLD = 12,
};

enum cpsspb_PHYInterfaceTypeOptions {
  // [SMI] Phy interface type smi
  cpsspb_PHYInterfaceTypeOptions_PHY_INTERFACE_TYPE_SMI = 0,
  // [XSMI] Phy interface type xsmi
  cpsspb_PHYInterfaceTypeOptions_PHY_INTERFACE_TYPE_XSMI = 1,
  // [Unused] Phy interface type unused
  cpsspb_PHYInterfaceTypeOptions_PHY_INTERFACE_TYPE_UNUSED = 2,
};

enum cpsspb_PTPSyncERecoveryClockSelectTypeOptions {
  // [Select Type 0] PTP sync e recovery clock select type 0
  cpsspb_PTPSyncERecoveryClockSelectTypeOptions_PTP_SYNC_E_RECOVERY_CLOCK_SELECT_TYPE_0 = 0,
  // [Select Type 1] PTP sync e recovery clock select type 1
  cpsspb_PTPSyncERecoveryClockSelectTypeOptions_PTP_SYNC_E_RECOVERY_CLOCK_SELECT_TYPE_1 = 1,
};

enum cpsspb_PTPPortDelayTypeOptions {
  cpsspb_PTPPortDelayTypeOptions_PTP_PORT_DELAY_INGRESS = 0,
  cpsspb_PTPPortDelayTypeOptions_PTP_PORT_DELAY_EGRESS = 1,
};

enum cpsspb_PTPModeTypeOptions {
  cpsspb_PTPModeTypeOptions_PTP_MODE_DISABLE = 0,
  cpsspb_PTPModeTypeOptions_PTP_MODE_E2E_TC = 1,
  cpsspb_PTPModeTypeOptions_PTP_MODE_P2P_TC = 2,
  cpsspb_PTPModeTypeOptions_PTP_MODE_MASTER_BC = 3,
  cpsspb_PTPModeTypeOptions_PTP_MODE_SLAVE_BC = 4,
  cpsspb_PTPModeTypeOptions_PTP_MODE_GPTP_MASTER = 5,
  cpsspb_PTPModeTypeOptions_PTP_MODE_GPTP_SLAVE = 6,
  cpsspb_PTPModeTypeOptions_PTP_MODE_PASSIVE = 7,
  cpsspb_PTPModeTypeOptions_PTP_MODE_LISTENING = 8,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct cpsspb_Enabled {
  bool IsEnabled;
};

struct cpsspb_IndexList {
  unsigned int List_Len; // auto-gen: for list
  int32_t *List;
};

struct cpsspb_MACAddress {
  char *Value;
};

struct cpsspb_DeviceArchitecture {
  struct cpsspb_DeviceInformation *Information;
  struct cpsspb_DevicePortAllocationTable *PortAllocationTable;
};

struct cpsspb_DeviceInformation {
  char *Model;
  char *MACAddress;
  char *Description;
  char *EnterpriseOID;
  char *Vendor;
  char *HardwareVersion;
  char *SoftwareVersion;
};

struct cpsspb_DevicePortProperty {
  int32_t PortNo;
  enum cpsspb_PortTypeOptions Type;
  int32_t MACNo;
  struct cpsspb_PHYInterface *PHYInterface;
  int32_t PHYID1;
  int32_t PHYID2;
  int32_t PHYChipID;
  int32_t PoENo;
  int32_t PoEChipNo;
  int32_t PoELEDNo;
  unsigned int SupportedTypeList_Len; // auto-gen: for list
  enum cpsspb_PortTypeOptions *SupportedTypeList;
  unsigned int SupportedMediaList_Len; // auto-gen: for list
  enum cpsspb_PortMediaOptions *SupportedMediaList;
};

struct cpsspb_PHYInterface {
  int32_t Interface;
  enum cpsspb_PHYInterfaceTypeOptions Type;
};

struct cpsspb_DevicePortAllocationTable {
  int32_t CPUPortNo;
  unsigned int PortPropertyList_Len; // auto-gen: for list
  struct cpsspb_DevicePortProperty **PortPropertyList;
  unsigned int PortList_Len; // auto-gen: for list
  int32_t *PortList;
  unsigned int MACNoList_Len; // auto-gen: for list
  int32_t *MACNoList;
  unsigned int PoEList_Len; // auto-gen: for list
  int32_t *PoEList;
  unsigned int SFPList_Len; // auto-gen: for list
  int32_t *SFPList;
  unsigned int LAGList_Len; // auto-gen: for list
  int32_t *LAGList;
};

struct cpsspb_DevicePortEntry {
  int32_t DeviceID;
  int32_t PortNo;
};

struct cpsspb_DevicePortList {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_DevicePortEntry **List;
};

struct cpsspb_DeviceTrunkEntry {
  int32_t DeviceID;
  int32_t TrunkID;
};

struct cpsspb_DeviceTrunkList {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_DeviceTrunkEntry **List;
};

struct cpsspb_DeviceInterfaceEntry {
  enum cpsspb_InterfaceTypeOptions Type;
  int32_t DeviceID;
  int32_t PortNo;
  int32_t LAGNo;
};

struct cpsspb_DeviceInterfaceList {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_DeviceInterfaceEntry **List;
};

struct cpsspb_TrunkingGroupMemberEntry {
  int32_t LAGNo;
  struct cpsspb_DevicePortList *EnableMemberList;
  struct cpsspb_DevicePortList *DisableMemberList;
};

struct cpsspb_TrunkingMemberEntry {
  int32_t LAGNo;
  struct cpsspb_DevicePortList *MemberList;
};

struct cpsspb_TrunkingGroupID {
  int32_t LAGNo;
};

struct cpsspb_TrunkingLoadBalanceMode {
  enum cpsspb_TrunkingLoadBalanceModeOptions Mode;
};

struct cpsspb_ACLIndexList {
  unsigned int List_Len; // auto-gen: for list
  int32_t *List;
};

struct cpsspb_ACLUserDefinedRuleList {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_ACLUserDefinedRuleEntry **List;
};

struct cpsspb_ACLUserDefinedRuleEntry {
  int32_t Index;
  char *UniqueID;
  enum cpsspb_ACLDirectionTypeOptions DirectionType;
  enum cpsspb_ACLRuleTypeOptions Type;
  char *SourceMAC;
  char *SourceMACMask;
  char *DestinationMAC;
  char *DestinationMACMask;
  int32_t EtherType;
  int32_t EtherTypeMask;
  char *SourceIPAddr;
  char *SourceIPMask;
  char *DestinationIPAddr;
  char *DestinationIPMask;
  int32_t IPProtocol;
  int32_t IPProtocolMask;
  int32_t L4SourcePort;
  int32_t L4SourcePortMask;
  int32_t L4DestinationPort;
  int32_t L4DestinationPortMask;
  int32_t VlanID;
  struct cpsspb_ACLSourceInterfaceEntry *SourceInterface;
  struct cpsspb_ACLActionEntry *Action;
};

struct cpsspb_ACLUserDefinedRuleRemovalList {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_ACLUserDefinedRuleRemovalEntry **List;
};

struct cpsspb_ACLUserDefinedRuleRemovalEntry {
  int32_t Index;
  char *UniqueID;
};

enum cpsspb_ACLControlRuleEntry_Rule_Union_Options {
  cpsspb_ACLControlRuleEntry_Rule_Union_Options_MACIPBindingRule,
  cpsspb_ACLControlRuleEntry_Rule_Union_Options_SubnetBasedRule,
  cpsspb_ACLControlRuleEntry_Rule_Union_Options_MACBasedRule,
  cpsspb_ACLControlRuleEntry_Rule_Union_Options_SelectiveQinQRule,
  cpsspb_ACLControlRuleEntry_Rule_Union_Options_FlowMirrorRule,
  cpsspb_ACLControlRuleEntry_Rule_Union_Options_MACCountingRule,
  cpsspb_ACLControlRuleEntry_Rule_Union_Options_PTPMessageRule,
};
struct cpsspb_ACLControlRuleEntry {
  enum cpsspb_ACLRuleTypeOptions Type;
  int32_t Index;
  struct cpsspb_ACLSourceInterfaceEntry *SourceInterface;
  struct cpsspb_ACLActionEntry *Action;
  enum cpsspb_ACLControlRuleEntry_Rule_Union_Options Rule_Union_Option;
  union {
    struct cpsspb_ACLMACIPBindingRuleEntry *Rule_MACIPBindingRule;
    struct cpsspb_ACLSubnetBasedRuleEntry *Rule_SubnetBasedRule;
    struct cpsspb_ACLMACBasedRuleEntry *Rule_MACBasedRule;
    struct cpsspb_ACLSelectiveQinQRuleEntry *Rule_SelectiveQinQRule;
    struct cpsspb_ACLFlowMirrorRuleEntry *Rule_FlowMirrorRule;
    struct cpsspb_ACLMACCountingRuleEntry *Rule_MACCountingRule;
    struct cpsspb_ACLPTPMessageRuleEntry *Rule_PTPMessageRule;
  } Rule;
};

struct cpsspb_ACLSourceInterfaceEntry {
  enum cpsspb_InterfaceTypeOptions Type;
  int32_t VlanID;
  unsigned int PortList_Len; // auto-gen: for list
  struct cpsspb_DevicePortEntry **PortList;
  unsigned int LAGList_Len; // auto-gen: for list
  struct cpsspb_TrunkingGroupID **LAGList;
};

struct cpsspb_ACLActionEntry {
  enum cpsspb_PacketCommandOptions PacketCommand;
};

struct cpsspb_ACLG8032RuleEntry {
  int32_t RingID;
  int32_t VlanID;
  int32_t DestinationRingPort;
};

struct cpsspb_ACLMACIPBindingRuleEntry {
  char *SourceMAC;
  char *SourceIPAddr;
};

struct cpsspb_ACLSubnetBasedRuleEntry {
  char *SourceIPAddr;
  char *SourceIPMask;
  int32_t ModifyVlanID;
};

struct cpsspb_ACLMACBasedRuleEntry {
  char *SourceMAC;
  char *SourceMACMask;
  int32_t ModifyVlanID;
};

struct cpsspb_ACLSelectiveQinQRuleEntry {
  int32_t VlanFrom;
  int32_t VlanTo;
};

struct cpsspb_ACLFlowMirrorRuleEntry {
  bool IsMirrorEnabled;
  struct cpsspb_ACLUserDefinedRuleEntry *Flow;
};

struct cpsspb_ACLMACCountingRuleEntry {
  int32_t CNCCounterIndex;
  char *SourceMAC;
};

struct cpsspb_ACLPTPMessageRuleEntry {
  int32_t EtherType;
};

enum cpsspb_CNCCountingEntry_Entry_Union_Options {
  cpsspb_CNCCountingEntry_Entry_Union_Options_MACAddressCountingEntry,
};
struct cpsspb_CNCCountingEntry {
  enum cpsspb_CNCCountingTypeOptions Type;
  enum cpsspb_CNCCountingEntry_Entry_Union_Options Entry_Union_Option;
  union {
    struct cpsspb_CNCMACAddressCountingEntry *Entry_MACAddressCountingEntry;
  } Entry;
};

struct cpsspb_CNCMACAddressCountingEntry {
  struct cpsspb_MACAddress *Address;
};

struct cpsspb_CNCCounter {
  uint64_t ByteCount;
  uint64_t PktCount;
};

struct cpsspb_FDBAutoLearningEnable {
  bool Enable;
};

struct cpsspb_FDBIdentifyEntry {
  int32_t VlanID;
  char *Address;
};

struct cpsspb_FDBSecureBreachEntry {
  struct cpsspb_DevicePortEntry *Port;
  bool Enable;
};

struct cpsspb_FDBMACEntry {
  enum cpsspb_InterfaceTypeOptions InterfaceType;
  bool IsStatic;
  bool IsForward;
  int32_t VlanID;
  char *Address;
  struct cpsspb_DevicePortEntry *Port;
  struct cpsspb_DeviceTrunkEntry *Trunk;
  struct cpsspb_FDBMulticastEntry *Multicast;
  enum cpsspb_FDBEntryActionTypeOptions Action;
};

struct cpsspb_FDBDumpEntry {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_FDBMACEntry **List;
};

struct cpsspb_FDBHashEntry {
  enum cpsspb_FDBLayerTypeOptions LayerType;
  int32_t VlanID;
  char *Address;
};

struct cpsspb_FDBAgingTime {
  int32_t Time;
};

struct cpsspb_FDBFlushEntry {
  enum cpsspb_InterfaceTypeOptions InterfaceType;
  bool IsUnicastDynamic;
  bool IsUnicastAll;
  bool IsMulticast;
  unsigned int VlanList_Len; // auto-gen: for list
  int32_t *VlanList;
  struct cpsspb_DevicePortList *PortList;
  struct cpsspb_DeviceTrunkList *TrunkList;
};

struct cpsspb_FDBMulticastEntry {
  enum cpsspb_MulticastEntryTypeOptions EntryType;
  int32_t VIDX;
};

struct cpsspb_FDBCounters {
  int32_t NumberOfFreeEntries;
  int32_t NumberOfUsedEntries;
  int32_t NumberOfMacUnicastDynamicEntries;
  int32_t NumberOfMacUnicastStaticEntries;
  int32_t NumberOfMacMulticastDynamicEntries;
  int32_t NumberOfMacMulticastStaticEntries;
  int32_t NumberOfIpv4MulticastEntries;
  int32_t NumberOfIpv6MulticastEntries;
};

struct cpsspb_MirroringSessionEntry {
  enum cpsspb_InterfaceTypeOptions SourceType;
  unsigned int SourcePortList_Len; // auto-gen: for list
  struct cpsspb_MirroringSourcePortEntry **SourcePortList;
  unsigned int SourceVlanList_Len; // auto-gen: for list
  struct cpsspb_MirroringSourceVlanEntry **SourceVlanList;
  struct cpsspb_DevicePortEntry *DestinationPort;
  struct cpsspb_MirroringRSPANEntry *RSPAN;
};

struct cpsspb_MirroringSourcePortEntry {
  enum cpsspb_MirroringDirectionTypeOptions Direction;
  struct cpsspb_DevicePortEntry *Port;
};

struct cpsspb_MirroringSourceVlanEntry {
  enum cpsspb_MirroringDirectionTypeOptions Direction;
  int32_t VlanID;
};

struct cpsspb_MirroringSessionList {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_MirroringSessionEntry **List;
};

struct cpsspb_MirroringPolicyBased {
  bool IsEnabled;
  struct cpsspb_DevicePortEntry *DestinationPort;
};

struct cpsspb_MirroringRSPANEntry {
  bool IsLocalEnabled;
  bool IsRemoteEnabled;
  int32_t AnalyzerVlanID;
};

struct cpsspb_MiscMACRegisterEntry {
  uint32_t Address;
  uint32_t Data;
};

struct cpsspb_MiscSMIRegisterEntry {
  uint32_t PHYInterface;
  uint32_t PHYID;
  uint32_t Address;
  uint32_t Data;
};

struct cpsspb_MiscXSMIRegisterEntry {
  uint32_t PHYInterface;
  uint32_t PHYID;
  uint32_t PHYDev;
  uint32_t Address;
  uint32_t Data;
};

struct cpsspb_MiscRegisterData {
  uint32_t Data;
};

struct cpsspb_MiscTemperature {
  int32_t Temperature;
};

struct cpsspb_MiscPHYPortModel {
  char *Interface;
  char *Model;
};

struct cpsspb_MiscModel {
  char *Model;
};

struct cpsspb_VlanID {
  int32_t VlanID;
};

struct cpsspb_VlanList {
  unsigned int List_Len; // auto-gen: for list
  int32_t *List;
};

struct cpsspb_VlanMemberTaggingList {
  int32_t VlanID;
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_VlanMemeberTaggingEntry **List;
};

struct cpsspb_VlanMemeberTaggingEntry {
  struct cpsspb_DeviceInterfaceEntry *Interface;
  enum cpsspb_VlanTaggingCommandTypeOptions Command;
};

struct cpsspb_VlanDefaultVIDEntry {
  int32_t VlanID;
  struct cpsspb_DeviceInterfaceEntry *Interface;
};

struct cpsspb_VlanAcceptFrameTypeEntry {
  struct cpsspb_DeviceInterfaceEntry *Interface;
  enum cpsspb_VlanAcceptFrameTypeOptions Type;
};

struct cpsspb_VlanTPIDIndexEntry {
  struct cpsspb_DeviceInterfaceEntry *Interface;
  int32_t Index;
};

struct cpsspb_VlanTPIDEntry {
  int32_t Index;
  int32_t TPID;
};

struct cpsspb_VlanTPIDList {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_VlanTPIDEntry **List;
};

struct cpsspb_VlanStatusEntry {
  int32_t VlanID;
  unsigned int UntaggedList_Len; // auto-gen: for list
  int32_t *UntaggedList;
  unsigned int TaggedList_Len; // auto-gen: for list
  int32_t *TaggedList;
  unsigned int ProviderList_Len; // auto-gen: for list
  int32_t *ProviderList;
  unsigned int CustomerList_Len; // auto-gen: for list
  int32_t *CustomerList;
};

struct cpsspb_VlanStatusList {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_VlanStatusEntry **List;
};

struct cpsspb_VlanFlushEntry {
  bool IsFlushAll;
  int32_t VlanID;
};

struct cpsspb_VlanTunnelEntry {
  struct cpsspb_DeviceInterfaceEntry *Interface;
  bool IsEnabled;
};

struct cpsspb_VlanProtocolClassID {
  int32_t ClassID;
};

struct cpsspb_VlanProtocolClassEntry {
  int32_t ClassID;
  int32_t EtherType;
  enum cpsspb_VlanProtocolBasedEncapsulationTypeOptions Encapsulation;
};

struct cpsspb_VlanProtocolEntry {
  struct cpsspb_DeviceInterfaceEntry *Interface;
  int32_t ClassID;
  int32_t VlanID;
};

struct cpsspb_VlanTranslationEntry {
  struct cpsspb_DeviceInterfaceEntry *Interface;
  bool IsEnabled;
};

struct cpsspb_VlanTranslationMappingEntry {
  int32_t SourceVlanID;
  int32_t TranslatedVlanID;
};

struct cpsspb_MulticastBridgeGroupEntry {
  struct cpsspb_FDBIdentifyEntry *Identify;
  unsigned int InterfaceList_Len; // auto-gen: for list
  struct cpsspb_DeviceInterfaceEntry **InterfaceList;
};

struct cpsspb_MulticastUnregisteredFloodingEntry {
  int32_t VlanID;
  unsigned int InterfaceList_Len; // auto-gen: for list
  struct cpsspb_DeviceInterfaceEntry **InterfaceList;
};

struct cpsspb_MulticastTableVlanFloodingEntry {
  int32_t VlanID;
  int32_t VIDX;
  unsigned int PortList_Len; // auto-gen: for list
  struct cpsspb_DevicePortEntry **PortList;
};

struct cpsspb_MulticastTableBridgeGroupEntry {
  int32_t VlanID;
  int32_t VIDX;
  char *Address;
  unsigned int PortList_Len; // auto-gen: for list
  struct cpsspb_DevicePortEntry **PortList;
};

struct cpsspb_MulticastTable {
  unsigned int VlanFloodingList_Len; // auto-gen: for list
  struct cpsspb_MulticastTableVlanFloodingEntry **VlanFloodingList;
  unsigned int BridgeGroupList_Len; // auto-gen: for list
  struct cpsspb_MulticastTableBridgeGroupEntry **BridgeGroupList;
};

struct cpsspb_PacketControlActionEntry {
  bool Enable;
  int32_t VlanID;
  struct cpsspb_DevicePortEntry *Port;
};

struct cpsspb_PacketControlGeneralStatusEntry {
  enum cpsspb_PacketControlProtocolTypeOptions Protocol;
  bool IsEnabled;
  enum cpsspb_PacketCommandOptions PacketCommand;
};

struct cpsspb_PacketControlInterfaceStatusEntry {
  enum cpsspb_PacketControlProtocolTypeOptions Protocol;
  enum cpsspb_InterfaceTypeOptions Type;
  unsigned int EnabledList_Len; // auto-gen: for list
  int32_t *EnabledList;
  enum cpsspb_PacketCommandOptions PacketCommand;
};

struct cpsspb_PacketControlStatusList {
  unsigned int GeneralList_Len; // auto-gen: for list
  struct cpsspb_PacketControlGeneralStatusEntry **GeneralList;
  unsigned int InterfaceList_Len; // auto-gen: for list
  struct cpsspb_PacketControlInterfaceStatusEntry **InterfaceList;
};

struct cpsspb_PolicerStormCountingEnableEntry {
  bool IsEnable;
  struct cpsspb_DevicePortEntry *Port;
};

struct cpsspb_PolicerStormCountingCounterEntry {
  uint64_t UnknownUnicastByteCount;
  uint64_t UnknownUnicastPktCount;
  uint64_t MulticastByteCount;
  uint64_t MulticastPktCount;
  uint64_t BroadcastByteCount;
  uint64_t BroadcastPktCount;
};

struct cpsspb_PortEnableEntry {
  struct cpsspb_DevicePortEntry *Port;
  bool Enable;
  enum cpsspb_PortSpeedDuplexOptions Speed;
};

struct cpsspb_PortSpeedDuplexEntry {
  struct cpsspb_DevicePortEntry *Port;
  enum cpsspb_PortSpeedDuplexOptions Speed;
};

struct cpsspb_PortSpeedDuplexList {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_PortSpeedDuplexEntry **List;
};

struct cpsspb_PortFlowControlEntry {
  struct cpsspb_DevicePortEntry *Port;
  bool Enable;
};

struct cpsspb_PortEEEEntry {
  struct cpsspb_DevicePortEntry *Port;
  bool Enable;
};

struct cpsspb_PortCounterEntry {
  struct cpsspb_DevicePortEntry *Port;
  uint64_t GoodOctetsRecv;
  uint64_t BadOctetsRecv;
  uint64_t MACTransmitErr;
  uint64_t GoodPktsRecv;
  uint64_t BadPktsRecv;
  uint64_t BrdcPktsRecv;
  uint64_t McPktsRecv;
  uint64_t Pkts64Octets;
  uint64_t Pkts65To127Octets;
  uint64_t Pkts128To255Octets;
  uint64_t Pkts256To511Octets;
  uint64_t Pkts512To1023Octets;
  uint64_t Pkts1024ToMaxOctets;
  uint64_t GoodOctetsSent;
  uint64_t GoodPktsSent;
  uint64_t ExcessiveCollisions;
  uint64_t McPktsSent;
  uint64_t BrdcPktsSent;
  uint64_t UnrecogMACCntrRecv;
  uint64_t FCSent;
  uint64_t GoodFCRecv;
  uint64_t DropEvents;
  uint64_t UndersizePkts;
  uint64_t FragmentsPkts;
  uint64_t OversizePkts;
  uint64_t JabberPkts;
  uint64_t MACRecvError;
  uint64_t BadCrc;
  uint64_t Collisions;
  uint64_t LateCollisions;
  uint64_t BadFcRecv;
  uint64_t UcPktsRecv;
  uint64_t UcPktsSent;
  uint64_t MultiplePktsSent;
  uint64_t DeferredPktsSent;
};

struct cpsspb_PortCounterList {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_PortCounterEntry **List;
};

struct cpsspb_PortIsolationEntry {
  struct cpsspb_DevicePortEntry *Port;
  unsigned int OutgoingList_Len; // auto-gen: for list
  struct cpsspb_DevicePortEntry **OutgoingList;
};

struct cpsspb_PortIsolationList {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_PortIsolationEntry **List;
};

struct cpsspb_PortIPGSize {
  int32_t Size;
};

struct cpsspb_PortFaultStatusEntry {
  bool IsRxFault;
  bool IsTxFault;
};

struct cpsspb_PortStatusEntry {
  struct cpsspb_DevicePortEntry *Port;
  bool IsEnabled;
  bool IsLinkUp;
  bool IsFlowControlEnabled;
  enum cpsspb_PortSpeedDuplexOptions LinkSpeed;
  bool EEEActive;
  // *internal usage
  char *PMState;
};

struct cpsspb_PTPToD {
  uint64_t Sec;
  uint32_t Ns;
};

struct cpsspb_PTPToDFrequency {
  uint32_t FractionalNs;
};

struct cpsspb_PTPQueueInfo {
  uint64_t Sec;
  uint32_t Ns;
};

struct cpsspb_PTPMessageInfo {
  struct cpsspb_DevicePortEntry *Port;
  uint32_t SeqID;
  uint32_t MessageType;
  uint32_t QueueNo;
};

struct cpsspb_PTPLinkDelay {
  struct cpsspb_DevicePortEntry *Port;
  int32_t Delay;
};

struct cpsspb_PTPPortDelay {
  struct cpsspb_DevicePortEntry *Port;
  enum cpsspb_PTPPortDelayTypeOptions Direction;
  int32_t Delay;
};

struct cpsspb_PTPMode {
  struct cpsspb_DevicePortEntry *Port;
  uint32_t DomainIndex;
  uint32_t DomainNumber;
  enum cpsspb_PTPModeTypeOptions Mode;
};

struct cpsspb_PTPSyncE {
  struct cpsspb_DevicePortEntry *Port;
  bool IsEnable;
  enum cpsspb_PortSpeedDuplexOptions Speed;
  enum cpsspb_PTPSyncERecoveryClockSelectTypeOptions ClockSelect;
};

struct cpsspb_QoSPortProfileEntry {
  struct cpsspb_DevicePortEntry *Port;
  enum cpsspb_QoSTrustModeOptions Mode;
};

struct cpsspb_QoSPortProfileList {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_QoSPortProfileEntry **List;
};

struct cpsspb_QoSPortQueueList {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_QoSPortQueueEntry **List;
};

struct cpsspb_QoSPortQueueEntry {
  int32_t QueueNo;
  int32_t Ratio;
  enum cpsspb_QoSSchedulerTypeOptions Scheduler;
};

struct cpsspb_QoSCoSProfile {
  int32_t CoSNo;
  int32_t QueueNo;
};

struct cpsspb_QoSCoSProfileList {
  unsigned int CoSList_Len; // auto-gen: for list
  struct cpsspb_QoSCoSProfile **CoSList;
};

struct cpsspb_QoSDSCPProfile {
  int32_t DSCPNo;
  int32_t QueueNo;
};

struct cpsspb_QoSDSCPProfileList {
  unsigned int DSCPList_Len; // auto-gen: for list
  struct cpsspb_QoSDSCPProfile **DSCPList;
};

struct cpsspb_RateLimitingIngressEntry {
  unsigned int FilterTypes_Len; // auto-gen: for list
  enum cpsspb_RateLimitingIngressFilterTypeOptions *FilterTypes;
  struct cpsspb_DevicePortEntry *Port;
  int32_t Rate;
};

struct cpsspb_RateLimitingIngressList {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_RateLimitingIngressEntry **List;
};

struct cpsspb_RateLimitingEgressEntry {
  struct cpsspb_DevicePortEntry *Port;
  int32_t Rate;
};

struct cpsspb_RateLimitingEgressList {
  unsigned int List_Len; // auto-gen: for list
  struct cpsspb_RateLimitingEgressEntry **List;
};

struct cpsspb_STPID {
  int32_t ID;
  enum cpsspb_STPProtocolTypeOptions Proto;
};

struct cpsspb_STPPortState {
  int32_t ID;
  struct cpsspb_DeviceInterfaceEntry *IdentifyNo;
  enum cpsspb_STPPortStateTypeOptions State;
  bool InActive;
  enum cpsspb_STPProtocolTypeOptions Proto;
};

struct cpsspb_STPVlanBinding {
  int32_t ID;
  enum cpsspb_STPProtocolTypeOptions Proto;
  unsigned int VlanList_Len; // auto-gen: for list
  int32_t *VlanList;
};

struct cpsspb_STPIDState {
  int32_t ID;
  enum cpsspb_STPPortStateTypeOptions State;
  enum cpsspb_STPProtocolTypeOptions Proto;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_cpss_cpss
