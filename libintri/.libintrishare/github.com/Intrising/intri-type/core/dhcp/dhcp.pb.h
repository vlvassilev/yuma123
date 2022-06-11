
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_dhcp_dhcp
#define _H_intri_pb_github_com_Intrising_intri_type_core_dhcp_dhcp

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>
#include <stdint.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum dhcppb_RemoteIDSourceTypeOptions {
  // [Hostname] Hostname of this switch
  dhcppb_RemoteIDSourceTypeOptions_REMOTE_ID_SOURCE_TYPE_HOSTNAME = 0,
  // [MacAddress] MAC address of this switch
  dhcppb_RemoteIDSourceTypeOptions_REMOTE_ID_SOURCE_TYPE_MAC_ADDRESS = 1,
  // [SysName] SNMP sysName of this switch
  dhcppb_RemoteIDSourceTypeOptions_REMOTE_ID_SOURCE_TYPE_SYS_NAME = 2,
  // [User defined] A user defined string as defined in custom remote ID is used
  dhcppb_RemoteIDSourceTypeOptions_REMOTE_ID_SOURCE_TYPE_USER_DEFINED = 3,
  // [Port alias] The port alias value of the incoming port is used
  dhcppb_RemoteIDSourceTypeOptions_REMOTE_ID_SOURCE_TYPE_PORT_ALIAS = 4,
};

enum dhcppb_CircuitIDSourceTypeOptions {
  // [Port alias] Use port alias as the circuit ID
  dhcppb_CircuitIDSourceTypeOptions_CIRCUIT_ID_SOURCE_TYPE_PORT_ALIAS = 0,
  // [IP-Port-VLAN] Use the format {IP}-{port}-{VLANID} as the circuit ID
  dhcppb_CircuitIDSourceTypeOptions_CIRCUIT_ID_SOURCE_TYPE_IP_PORT_VLAN = 1,
};

enum dhcppb_SnoopingPortDHCPFilteringTypeOptions {
  // [Disabled] DHCP frames are not removed
  dhcppb_SnoopingPortDHCPFilteringTypeOptions_SNOOPING_PORT_DHCP_FILTERING_TYPE_DISABLED = 0,
  // [Drop and event] DHCP response frames incoming from a user port are removed and a PACKET_INTERCEPTED event is send
  dhcppb_SnoopingPortDHCPFilteringTypeOptions_SNOOPING_PORT_DHCP_FILTERING_TYPE_DROP_AND_EVENT = 1,
  // [Block and event] Port is blocked when an DHCP response incoming from a user port is detected. Needs operator intervention to unblock. Also a PACKET_INTERCEPTED event is send.
  dhcppb_SnoopingPortDHCPFilteringTypeOptions_SNOOPING_PORT_DHCP_FILTERING_BLOCK_AND_EVENT = 2,
};

enum dhcppb_SnoopingPortSnoopingTrustTypeOptions {
  // [Untrusted] This port is untrusted and DHCP filtering applies
  dhcppb_SnoopingPortSnoopingTrustTypeOptions_SNOOPING_PORT_SNOOPING_TYPE_TRUST_UNTRUSTED = 0,
  // [Trusted] This port is trusted and no filtering occurs. Use when a DHCP server should be permitted on a local access port
  dhcppb_SnoopingPortSnoopingTrustTypeOptions_SNOOPING_PORT_SNOOPING_TYPE_TRUST_TRUSTED = 1,
};

enum dhcppb_SnoopingLastDropReasonTypeOptions {
  // [Ok] The packet is normal and will be forwarded
  dhcppb_SnoopingLastDropReasonTypeOptions_SNOOPING_LAST_DROP_REASON_TYPE_OK = 0,
  // [Illegal DHCP Server] Forbidden DHCP message on untrusted port
  dhcppb_SnoopingLastDropReasonTypeOptions_SNOOPING_LAST_DROP_REASON_TYPE_ILLEGAL_DHCP_SERVER = 1,
  // [DHCP Server Sproofed] Source MAC and DHCP client MAC did not match
  dhcppb_SnoopingLastDropReasonTypeOptions_SNOOPING_LAST_DROP_REASON_TYPE_DHCP_SERVER_SPOOFED = 2,
  // [DHCP Illegal Relay Agent]
  dhcppb_SnoopingLastDropReasonTypeOptions_SNOOPING_LAST_DROP_REASON_TYPE_ILLEGAL_RELAY_AGENT = 3,
  // [DHCP Binding mismatch] DHCPRELEASE or DHCPDECLINE interface information did not match the binding table information
  dhcppb_SnoopingLastDropReasonTypeOptions_SNOOPING_LAST_DROP_REASON_TYPE_BINDING_MISMATCH = 4,
  // [Flooding] Too many DHCP messages which appears to be an attack.
  dhcppb_SnoopingLastDropReasonTypeOptions_SNOOPING_LAST_DROP_REASON_TYPE_FLOODING = 5,
  // [DHCP Filtering] DHCP reponse is dropped by DHCP Filtering
  dhcppb_SnoopingLastDropReasonTypeOptions_SNOOPING_LAST_DROP_REASON_TYPE_DHCP_FILTERING = 6,
};

enum dhcppb_ARPInspectionDatabaseTypeOptions {
  // [None] Don't use any database
  dhcppb_ARPInspectionDatabaseTypeOptions_ARP_INSPECTION_DATABASE_TYPE_NONE = 0,
  // [DHCP] Use the DHCP snooping binding database as the arp inspection database
  dhcppb_ARPInspectionDatabaseTypeOptions_ARP_INSPECTION_DATABASE_TYPE_DHCP = 1,
  // [ACL] Use the ACL list as the arp inspection database
  dhcppb_ARPInspectionDatabaseTypeOptions_ARP_INSPECTION_DATABASE_TYPE_ARP_ACL = 2,
  // [Both] Use both DHCP and ACL
  dhcppb_ARPInspectionDatabaseTypeOptions_ARP_INSPECTION_DATABASE_TYPE_BOTH = 3,
};

// This is the default action if none of the rules in this entry is matched.
enum dhcppb_ARPInspectionACLRuleUnmatchedActionTypeOptions {
  // [Deny] The packet will be denied
  dhcppb_ARPInspectionACLRuleUnmatchedActionTypeOptions_ARP_INSPECTION_ACL_RULE_UNMATCHED_ACTION_TYPE_DENY = 0,
  // [Permit] The packet will be passed
  dhcppb_ARPInspectionACLRuleUnmatchedActionTypeOptions_ARP_INSPECTION_ACL_RULE_UNMATCHED_ACTION_TYPE_PERMIT = 1,
};

// This is for the rule inside the ARPInspectionACLRuleEntry, an entry may contain many rules
enum dhcppb_ARPInspectionACLRuleModeTypeOptions {
  // [Unused] The ACL rule entry won't use this rule
  dhcppb_ARPInspectionACLRuleModeTypeOptions_ARP_INSPECTION_RULE_MODE_TYPE_UNUSED = 0,
  // [Permit] The ACL rule entry will permit the packet if this mode is permitted
  dhcppb_ARPInspectionACLRuleModeTypeOptions_ARP_INSPECTION_RULE_MODE_TYPE_PERMIT = 1,
  // [Deny] The ACL rule entry will deny the packet if this mode is denied
  dhcppb_ARPInspectionACLRuleModeTypeOptions_ARP_INSPECTION_RULE_MODE_TYPE_DENY = 2,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct dhcppb_ARPInspectionConfig {
  bool Enabled;
  unsigned int Ports_Len; // auto-gen: for list
  struct dhcppb_ARPInspectionPortEntry **Ports;
};

struct dhcppb_ARPInspectionPortEntry {
  // pre-generated, the unique key for update the config, phy port and lg port
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool Enabled;
  // 0 means to disable the rate limiting
  int32_t RateLimiting;
  enum dhcppb_ARPInspectionDatabaseTypeOptions Database;
  struct dhcppb_ARPInspectionACLConfig *ACL;
  // Check the srcMAC is the senderMAC
  bool SrcMACValid;
  // Check the dstMAC is the targetMAC when the pkt is ARP response
  bool DstMACValid;
  // Checks for invalid IP including 0.0.0.0, 255.255.255.255, and all multicast and loopback addresses. Sender IP are checked in all ARP requests and responses, and target IP are checked only in ARP responses.
  bool IPRangeValid;
};

struct dhcppb_ARPInspectionACLConfig {
  // repeated ARPInspectionACLRuleEntry rules = 1 [deprecated = true];
// This field represents the action will be taken if there is no ARPInspectionACLRuleEntry is matched
  enum dhcppb_ARPInspectionACLRuleUnmatchedActionTypeOptions ACLRuleUnmatchedAction;
  // This field should contains the name of the acl.ACEEntry that exists in the acl.ACEList
  unsigned int Rules_Len; // auto-gen: for list
  char **Rules;
};

struct dhcppb_ARPInspectionACLRuleEntry {
  // The index for this entry is vid+srcMac+srcIP+srcIPMask
// https://github.com/Intrising/test-switch/issues/2615
  int32_t VID;
  char *SrcMAC;
  char *SrcIP;
  char *SrcIPMask;
  enum dhcppb_ARPInspectionACLRuleModeTypeOptions Mode;
};

struct dhcppb_Config {
  struct dhcppb_RelayConfig *Relay;
  struct dhcppb_SnoopingConfig *Snooping;
};

struct dhcppb_RelayConfig {
  // Enable global DHCP relay function
  bool Enabled;
  unsigned int RelayPorts_Len; // auto-gen: for list
  struct dhcppb_ConfigRelayPortEntry **RelayPorts;
  //  server ipaddress is used to let relay know which server to forward the pkt
  char *DHCPServerAddress;
  // System will auto insert the RemoteID to the Option82 packet following this source option
  enum dhcppb_RemoteIDSourceTypeOptions RemoteIDSource;
  // If the remote source iD option is custom, then the value in this field will be inserted
  char *CustomRemoteID;
  // System will auto insert the circuitID to the Option82 packet following this source option
  enum dhcppb_CircuitIDSourceTypeOptions CircuitIDSource;
};

struct dhcppb_ConfigRelayPortEntry {
  // The pre-generated id to update the config, the device.InterfaceIdentify of this field will include physical port and logical port
  struct devicepb_InterfaceIdentify *IdentifyNo;
  // Enable relay on this port
  bool Enabled;
  // Enable option82 on this port
  bool Option82Enabled;
};

struct dhcppb_SnoopingConfig {
  // Enable global DHCP snooping function
  bool Enabled;
  unsigned int Ports_Len; // auto-gen: for list
  struct dhcppb_SnoopingConfigPortEntry **Ports;
};

struct dhcppb_SnoopingConfigPortEntry {
  // The pre-generated id to update the config, the device.InterfaceIdentify of this field will include physical port and logical port
  struct devicepb_InterfaceIdentify *IdentifyNo;
  // Enable snooping on this port
  bool Enabled;
  enum dhcppb_SnoopingPortDHCPFilteringTypeOptions DHCPFiltering;
  enum dhcppb_SnoopingPortSnoopingTrustTypeOptions SnoopingTrust;
  // Enable this field and Snooping will process the pkt with Option82, othewise will drop the pkt
  bool AcceptIngressOption82;
  bool MACAddressVerification;
  // This field defines the amount that DHCP pkt could pass per second, set it to 0 to disable this function
  int32_t DHCPRateLimiting;
};

struct dhcppb_SnoopingStatisticsEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  enum dhcppb_SnoopingPortSnoopingTrustTypeOptions TrustMode;
  int32_t Processed;
  int32_t Dropped;
  enum dhcppb_SnoopingLastDropReasonTypeOptions LastDropReason;
};

struct dhcppb_SnoopingStatisticsList {
  unsigned int List_Len; // auto-gen: for list
  struct dhcppb_SnoopingStatisticsEntry **List;
};

// change the naming from SnoopingTable to SnoopingBindingDatabase, just like CISCO
struct dhcppb_SnoopingBindingDatabaseEntry {
  char *MACAddress;
  char *IP;
  struct devicepb_InterfaceIdentify *IdentifyNo;
  // https://github.com/Intrising/test-switch/issues/2615
  int32_t VID;
  char *LastUpdated;
  char *LastUpdatedEpoch;
  int64_t LeaseTime;
};

struct dhcppb_SnoopingBindingDatabaseList {
  unsigned int List_Len; // auto-gen: for list
  struct dhcppb_SnoopingBindingDatabaseEntry **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_dhcp_dhcp
