
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_lldp_lldp
#define _H_intri_pb_github_com_Intrising_intri_type_core_lldp_lldp

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

enum lldppb_PortModeTypeOptions {
  // [Disabled] Set port disabled
  lldppb_PortModeTypeOptions_PORT_MODE_TYPE_DISABLED = 0,
  // [TX only] Set port only transmit the packet
  lldppb_PortModeTypeOptions_PORT_MODE_TYPE_ENABLED_TX_ONLY = 1,
  // [RX only] Set port only receive the packet
  lldppb_PortModeTypeOptions_PORT_MODE_TYPE_ENABLED_RX_ONLY = 2,
  // [Both] Set port to receive and transmit the packet
  lldppb_PortModeTypeOptions_PORT_MODE_TYPE_ENABLED_TX_RX = 3,
};

enum lldppb_CfgAdvertizedMedClassTypeOptions {
  // [Disable MED]
  lldppb_CfgAdvertizedMedClassTypeOptions_CFG_ADVERTIZED_MED_CLASS_TYPE_DISABLE_MED = 0,
  // [Generic End Point]
  lldppb_CfgAdvertizedMedClassTypeOptions_CFG_ADVERTIZED_MED_CLASS_TYPE_GENERIC_ENDPOINT = 1,
  // [Media END Point]
  lldppb_CfgAdvertizedMedClassTypeOptions_CFG_ADVERTIZED_MED_CLASS_TYPE_MEDIA_ENDPOINT = 2,
  // [Communication End Point]
  lldppb_CfgAdvertizedMedClassTypeOptions_CFG_ADVERTIZED_MED_CLASS_TYPE_COMMUNICATION_ENDPOINT = 3,
  // [Network Connectivity Device]
  lldppb_CfgAdvertizedMedClassTypeOptions_CFG_ADVERTIZED_MED_CLASS_TYPE_NETWORK_DEVICE = 4,
};

enum lldppb_MACPHYAdvertisedCapabilityTypeOptions {
  // [Base 1000T Full]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_BASE1000_T_FULL = 0,
  // [Base 1000T Half]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_BASE1000_T_HALF = 1,
  // [Base 1000X Full]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_BASE1000_X_FULL = 2,
  // [Base 1000X Half]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_BASE1000_X_HALF = 3,
  // [ASYM SYM Pause]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_ASYM_SYM_PAUSE = 4,
  // [SYM Pause]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_SYM_PAUSE = 5,
  // [ASYM Pause]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_ASYM_PAUSE = 6,
  // [Pause]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_PAUSE = 7,
  // [Base 100T2 Full]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_BASE100_T2_FULL = 8,
  // [Base 100T2 Half]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_BASE100_T2_HALF = 9,
  // [Base 100TX Full]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_BASE100_TX_FULL = 10,
  // [Base 100TX Half]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_BASE100_TX_HALF = 11,
  // [Base 100T4]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_BASE100_T4 = 12,
  // [Base 10T Full]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_BASE10_T_FULL = 13,
  // [Base 10T Half]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_BASE10_T_HALF = 14,
  // [Other or unknown]
  lldppb_MACPHYAdvertisedCapabilityTypeOptions_MAC_PHY_ADVERTIZED_CAPABILITY_TYPE_OTHER_OR_UNKNOWN = 15,
};

enum lldppb_MedCapabilitiesTypeOptions {
  // [Capability]
  lldppb_MedCapabilitiesTypeOptions_MED_CAPABILITIES_TYPE_CAPABILITY = 0,
  // [Policy]
  lldppb_MedCapabilitiesTypeOptions_MED_CAPABILITIES_TYPE_POLICY = 1,
  // [Location]
  lldppb_MedCapabilitiesTypeOptions_MED_CAPABILITIES_TYPE_LOCATION = 2,
  // [MDI PSE]
  lldppb_MedCapabilitiesTypeOptions_MED_CAPABILITIES_TYPE_MDI_PSE = 3,
  // [MDI PD]
  lldppb_MedCapabilitiesTypeOptions_MED_CAPABILITIES_TYPE_MDI_PD = 4,
  // [Inventory]
  lldppb_MedCapabilitiesTypeOptions_MED_CAPABILITIES_TYPE_INVENTORY = 5,
};

enum lldppb_AltitudeTypeOptions {
  // [Meter]
  lldppb_AltitudeTypeOptions_ALTITUDE_TYPE_METER = 0,
  // [Floor]
  lldppb_AltitudeTypeOptions_ALTITUDE_TYPE_FLOOR = 1,
};

enum lldppb_ChassisIdSubtypeTypeOptions {
  // [Reserved]
  lldppb_ChassisIdSubtypeTypeOptions_CHASSIS_ID_SUBTYPE_TYPE_RESERVED = 0,
  // [Chassis Component]
  lldppb_ChassisIdSubtypeTypeOptions_CHASSIS_ID_SUBTYPE_TYPE_CHASSIS_COMPONENT = 1,
  // [Interface Alias]
  lldppb_ChassisIdSubtypeTypeOptions_CHASSIS_ID_SUBTYPE_TYPE_INTERFACE_ALIAS = 2,
  // [Port Componenet]
  lldppb_ChassisIdSubtypeTypeOptions_CHASSIS_ID_SUBTYPE_TYPE_PORT_COMPONENT = 3,
  // [MAC Address]
  lldppb_ChassisIdSubtypeTypeOptions_CHASSIS_ID_SUBTYPE_TYPE_MAC_ADDRESS = 4,
  // [Network Address]
  lldppb_ChassisIdSubtypeTypeOptions_CHASSIS_ID_SUBTYPE_TYPE_NETWORK_ADDRESS = 5,
  // [Interface Name]
  lldppb_ChassisIdSubtypeTypeOptions_CHASSIS_ID_SUBTYPE_TYPE_INTERFACE_NAME = 6,
  // [Local]
  lldppb_ChassisIdSubtypeTypeOptions_CHASSIS_ID_SUBTYPE_TYPE_LOCAL = 7,
};

enum lldppb_CapabilitiesTypeOptions {
  // [Other]
  lldppb_CapabilitiesTypeOptions_CAPABILITIES_TYPE_OTHER = 0,
  // [Repeater]
  lldppb_CapabilitiesTypeOptions_CAPABILITIES_TYPE_REPEATER = 1,
  // [Bridge]
  lldppb_CapabilitiesTypeOptions_CAPABILITIES_TYPE_BRIDGE = 2,
  // [WLAN]
  lldppb_CapabilitiesTypeOptions_CAPABILITIES_TYPE_WLAN = 3,
  // [Router]
  lldppb_CapabilitiesTypeOptions_CAPABILITIES_TYPE_ROUTER = 4,
  // [Telephone]
  lldppb_CapabilitiesTypeOptions_CAPABILITIES_TYPE_TELEPHONE = 5,
  // [DOCSIS]
  lldppb_CapabilitiesTypeOptions_CAPABILITIES_TYPE_DOCSIS = 6,
  // [Station]
  lldppb_CapabilitiesTypeOptions_CAPABILITIES_TYPE_STATION = 7,
};

enum lldppb_ManagementAddressIfSubTypeTypeOptions {
  // *[dont use]
  lldppb_ManagementAddressIfSubTypeTypeOptions_MANAGEMENT_ADDRESS_IF_SUBTYPE_TYPE_DONT_USE = 0,
  // [Unknown]
  lldppb_ManagementAddressIfSubTypeTypeOptions_MANAGEMENT_ADDRESS_IF_SUBTYPE_TYPE_UNKNOWN = 1,
  // [IfIndex]
  lldppb_ManagementAddressIfSubTypeTypeOptions_MANAGEMENT_ADDRESS_IF_SUBTYPE_TYPE_IF_INDEX = 2,
  // [System Port number]
  lldppb_ManagementAddressIfSubTypeTypeOptions_MANAGEMENT_ADDRESS_IF_SUBTYPE_TYPE_SYSTEM_PORT_NUMBER = 3,
};

enum lldppb_PortIdSubtypeTypeOptions {
  // [Unknown]
  lldppb_PortIdSubtypeTypeOptions_PORT_ID_SUBTYPE_TYPE_UNKNOWN = 0,
  // [Interface Alias]
  lldppb_PortIdSubtypeTypeOptions_PORT_ID_SUBTYPE_TYPE_INTERFACE_ALIAS = 1,
  // [Port Component]
  lldppb_PortIdSubtypeTypeOptions_PORT_ID_SUBTYPE_TYPE_PORT_COMPONENT = 2,
  // [MAC Address]
  lldppb_PortIdSubtypeTypeOptions_PORT_ID_SUBTYPE_TYPE_MAC_ADDRESS = 3,
  // [Network Address]
  lldppb_PortIdSubtypeTypeOptions_PORT_ID_SUBTYPE_TYPE_NETWORK_ADDRESS = 4,
  // [Interface Name]
  lldppb_PortIdSubtypeTypeOptions_PORT_ID_SUBTYPE_TYPE_INTERFACE_NAME = 5,
  // [Agent Circuit ID]
  lldppb_PortIdSubtypeTypeOptions_PORT_ID_SUBTYPE_TYPE_AGENT_CIRCUIT_ID = 6,
  // [Local]
  lldppb_PortIdSubtypeTypeOptions_PORT_ID_SUBTYPE_TYPE_LOCAL = 7,
};

enum lldppb_PoliciesApplicationTypeOptions {
  // [Unknown] Unspecified application
  lldppb_PoliciesApplicationTypeOptions_POLICIES_APPLICATION_TYPE_UNKNOWN = 0,
  // [Voice] Used by dedicated IP phone handsets and other similar devices supporting interactive voice services
  lldppb_PoliciesApplicationTypeOptions_POLICIES_APPLICATION_TYPE_VOICE = 1,
  // [Voice Signaling] Defines a separate policy for the command and control signaling that supports voice applications
  lldppb_PoliciesApplicationTypeOptions_POLICIES_APPLICATION_TYPE_VOICE_SIGNALING = 2,
  // [Guest Voice] Limited feature-set voice service for guest users
  lldppb_PoliciesApplicationTypeOptions_POLICIES_APPLICATION_TYPE_GUEST_VOICE = 3,
  // [Guest Voice Signaling] Defines a separate policy for the command and control signaling that supports guest voice applications
  lldppb_PoliciesApplicationTypeOptions_POLICIES_APPLICATION_TYPE_GUEST_VOICE_SIGNALING = 4,
  // [Soft Phone Voice] Used by softphone applications that operate on devices, such as PCs or laptop computers
  lldppb_PoliciesApplicationTypeOptions_POLICIES_APPLICATION_TYPE_SOFTPHONE_VOICE = 5,
  // [Video Conerencing] Used by video conferencing applications
  lldppb_PoliciesApplicationTypeOptions_POLICIES_APPLICATION_TYPE_VIDEO_CONFERENCING = 6,
  // [Streaming Video] Used for streaming video applications
  lldppb_PoliciesApplicationTypeOptions_POLICIES_APPLICATION_TYPE_STREAMING_VIDEO = 7,
  // [Video Signaling] Defines a separate policy for the command and control of video applications
  lldppb_PoliciesApplicationTypeOptions_POLICIES_APPLICATION_TYPE_VIDEO_SIGNALING = 8,
};

enum lldppb_PoliciesLayer2PriorityTypeOptions {
  // [Best Effort]
  lldppb_PoliciesLayer2PriorityTypeOptions_POLICIES_LAYER2_PRIORITY_TYPE_BEST_EFFORT = 0,
  // [Unknown]
  lldppb_PoliciesLayer2PriorityTypeOptions_POLICIES_LAYER2_PRIORITY_TYPE_UNKNOWN = 1,
  // [Background]
  lldppb_PoliciesLayer2PriorityTypeOptions_POLICIES_LAYER2_PRIORITY_TYPE_BACKGROUND = 2,
  // [Spare]
  lldppb_PoliciesLayer2PriorityTypeOptions_POLICIES_LAYER2_PRIORITY_TYPE_SPARE = 3,
  // [Excellent Effort]
  lldppb_PoliciesLayer2PriorityTypeOptions_POLICIES_LAYER2_PRIORITY_TYPE_EXCELLENT_EFFORT = 4,
  // [Controlled Load]
  lldppb_PoliciesLayer2PriorityTypeOptions_POLICIES_LAYER2_PRIORITY_TYPE_CONTROLLED_LOAD = 5,
  // [Video]
  lldppb_PoliciesLayer2PriorityTypeOptions_POLICIES_LAYER2_PRIORITY_TYPE_VIDEO = 6,
  // [Voice]
  lldppb_PoliciesLayer2PriorityTypeOptions_POLICIES_LAYER2_PRIORITY_TYPE_VOICE = 7,
  // [Network Control]
  lldppb_PoliciesLayer2PriorityTypeOptions_POLICIES_LAYER2_PRIORITY_TYPE_NETWORK_CONTROL = 8,
};

enum lldppb_PoePortClassTypeOptions {
  // [PD]
  lldppb_PoePortClassTypeOptions_POE_PORT_CLASS_TYPE_PD = 0,
  // [PSE]
  lldppb_PoePortClassTypeOptions_POE_PORT_CLASS_TYPE_PSE = 1,
};

enum lldppb_PoeInfoTypeOptions {
  // [Type2 PSE]
  lldppb_PoeInfoTypeOptions_POE_INFO_TYPE_2_PSE_DEVICE = 0,
  // [Type2 PD]
  lldppb_PoeInfoTypeOptions_POE_INFO_TYPE_2_PD_DEVICE = 1,
  // [Type1 PSE]
  lldppb_PoeInfoTypeOptions_POE_INFO_TYPE_1_PSE_DEVICE = 2,
  // [Type1 PD]
  lldppb_PoeInfoTypeOptions_POE_INFO_TYPE_1_PD_DEVICE = 3,
};

enum lldppb_PoeInfoSourceTypeOptions {
  // [Unknown] No information received
  lldppb_PoeInfoSourceTypeOptions_POE_INFO_SOURCE_TYPE_UNKNOWN = 0,
  // [PD PSE Primary] For type PD: Power source is the PSE. For type PSE: Power source is the primary power source
  lldppb_PoeInfoSourceTypeOptions_POE_INFO_SOURCE_TYPE_PD_PSE_PRIMARY = 1,
  // [PD Local Backup] For type PD: Power source is a local source. For type PSE: Power source is the backup power source
  lldppb_PoeInfoSourceTypeOptions_POE_INFO_SOURCE_TYPE_PD_LOCAL_BACKUP = 2,
  // [PD PSE Local] For type PD: The power source is both the PSE and a local source. For type PSE: this value should not occur
  lldppb_PoeInfoSourceTypeOptions_POE_INFO_SOURCE_TYPE_PD_PSE_LOCAL = 3,
};

enum lldppb_PoeInfoPriorityTypeOptions {
  // [Unknown] No information received
  lldppb_PoeInfoPriorityTypeOptions_POE_INFO_PRIORITY_TYPE_UNKNOWN = 0,
  // [Critical] Critical priority
  lldppb_PoeInfoPriorityTypeOptions_POE_INFO_PRIORITY_TYPE_CRITICAL = 1,
  // [High] High priority
  lldppb_PoeInfoPriorityTypeOptions_POE_INFO_PRIORITY_TYPE_HIGH = 2,
  // [Low] Low priority
  lldppb_PoeInfoPriorityTypeOptions_POE_INFO_PRIORITY_TYPE_LOW = 3,
};

enum lldppb_PoeControlPowerPairsTypeOptions {
  // [Signal]
  lldppb_PoeControlPowerPairsTypeOptions_POE_CONTROL_POWER_PAIRS_TYPE_SIGNAL = 0,
  // [Spare]
  lldppb_PoeControlPowerPairsTypeOptions_POE_CONTROL_POWER_PAIRS_TYPE_SPARE = 1,
};

enum lldppb_PoeControlPowerClassTypeOptions {
  // [None]
  lldppb_PoeControlPowerClassTypeOptions_POE_CONTROL_POWER_CLASS_TYPE_NONE = 0,
  // [Type0]
  lldppb_PoeControlPowerClassTypeOptions_POE_CONTROL_POWER_CLASS_TYPE_0 = 1,
  // [Type1]
  lldppb_PoeControlPowerClassTypeOptions_POE_CONTROL_POWER_CLASS_TYPE_1 = 2,
  // [Type2]
  lldppb_PoeControlPowerClassTypeOptions_POE_CONTROL_POWER_CLASS_TYPE_2 = 3,
  // [Type3]
  lldppb_PoeControlPowerClassTypeOptions_POE_CONTROL_POWER_CLASS_TYPE_3 = 4,
  // [Type4]
  lldppb_PoeControlPowerClassTypeOptions_POE_CONTROL_POWER_CLASS_TYPE_4 = 5,
  // [Type5]
  lldppb_PoeControlPowerClassTypeOptions_POE_CONTROL_POWER_CLASS_TYPE_5 = 6,
  // [Type6]
  lldppb_PoeControlPowerClassTypeOptions_POE_CONTROL_POWER_CLASS_TYPE_6 = 7,
  // [Type7]
  lldppb_PoeControlPowerClassTypeOptions_POE_CONTROL_POWER_CLASS_TYPE_7 = 8,
  // [Type8]
  lldppb_PoeControlPowerClassTypeOptions_POE_CONTROL_POWER_CLASS_TYPE_8 = 9,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct lldppb_Config {
  struct lldppb_SystemConfig *SystemConfig;
  struct lldppb_PortConfig *PortConfig;
};

struct lldppb_SystemConfig {
  // Default value is false
  bool Enabled;
  // Default value is 128, the unit of this field is second
  int32_t TimeToLive;
  // Default value is 30, the unit of this field is second
  int32_t MsgTxInterval;
  // Default value is false
  bool VoiceDisableVlanTLV;
  // Default value is false
  bool ForwardToLink;
};

struct lldppb_PortConfig {
  unsigned int List_Len; // auto-gen: for list
  struct lldppb_PortConfigEntry **List;
};

struct lldppb_PortConfigEntry {
  // Index to update, only support Physical port
  struct devicepb_InterfaceIdentify *IdentifyNo;
  // Default value is PORT_MODE_TYPE_ENABLED_TX_RX
  enum lldppb_PortModeTypeOptions Mode;
};

struct lldppb_LocalInfo {
  unsigned int List_Len; // auto-gen: for list
  struct lldppb_LocalInfoEntry **List;
};

struct lldppb_NeighborInfo {
  unsigned int List_Len; // auto-gen: for list
  struct lldppb_NeighborInfoEntry **List;
};

struct lldppb_LocalInfoEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  struct lldppb_MACPHYConfig *MACPhy;
  struct lldppb_PortManagementInfo *PortMgmt;
  struct lldppb_LinkAggregation *Aggr;
  struct lldppb_ExtendedPowerViaMDI *ExtendPoe;
  struct lldppb_PowerViaMDI *Power;
  struct lldppb_MediaCapability *MediaCap;
  struct lldppb_SystemManagementInfo *SystemInfo;
};

struct lldppb_NeighborInfoEntry {
  struct devicepb_InterfaceIdentify *RecvIdentifyNo;
  int32_t TimeToLive;
  struct lldppb_ChassisInfo *Chassis;
  unsigned int MgmtAddress_Len; // auto-gen: for list
  struct lldppb_PortManagementAddressInfo **MgmtAddress;
  unsigned int Capabilities_Len; // auto-gen: for list
  enum lldppb_CapabilitiesTypeOptions *Capabilities;
  unsigned int CapabilitiesEnabled_Len; // auto-gen: for list
  enum lldppb_CapabilitiesTypeOptions *CapabilitiesEnabled;
  unsigned int MedCapabilities_Len; // auto-gen: for list
  enum lldppb_MedCapabilitiesTypeOptions *MedCapabilities;
  struct lldppb_PortID *PortSubtype;
  unsigned int VoiceVlan_Len; // auto-gen: for list
  struct lldppb_VoiceVlanEntry **VoiceVlan;
  struct lldppb_SystemManagementInfo *SystemInfo;
  struct lldppb_ExtendedPowerViaMDI *ExtendPoe;
  struct lldppb_PowerViaMDI *Power;
};

struct lldppb_LinkAggregation {
  bool Capabilities;
  bool Status;
  int32_t AggregationPortNo;
};

struct lldppb_ExtendedPowerViaMDI {
  enum lldppb_PoeInfoTypeOptions Type;
  enum lldppb_PoeInfoSourceTypeOptions Source;
  enum lldppb_PoeInfoPriorityTypeOptions Priority;
  float Value;
};

struct lldppb_MACPHYConfig {
  bool AutoNegotiationConfig;
  bool AutoNegotiationStatus;
  unsigned int AutoNegoAdvertisedCapability_Len; // auto-gen: for list
  enum lldppb_MACPHYAdvertisedCapabilityTypeOptions *AutoNegoAdvertisedCapability;
  enum lldppb_MACPHYAdvertisedCapabilityTypeOptions OperationalMAUType;
};

struct lldppb_MediaCapability {
  unsigned int Capabilities_Len; // auto-gen: for list
  enum lldppb_MedCapabilitiesTypeOptions *Capabilities;
  int32_t ClassType;
};

struct lldppb_SystemManagementInfo {
  char *Name;
  char *Description;
};

struct lldppb_PortManagementInfo {
  unsigned int Ip_Len; // auto-gen: for list
  char **Ip;
  char *MACAddr;
  char *Description;
};

struct lldppb_PowerViaMDI {
  enum lldppb_PoePortClassTypeOptions Type;
  bool PoePowerSupported;
  bool PoePowerEnabled;
  bool PairControl;
  enum lldppb_PoeControlPowerPairsTypeOptions PowerPairs;
  enum lldppb_PoeControlPowerClassTypeOptions PowerClass;
  enum lldppb_PoeInfoTypeOptions DeviceType;
  enum lldppb_PoeInfoSourceTypeOptions Source;
  enum lldppb_PoeInfoPriorityTypeOptions Priority;
  float PdRequestedPower;
  float PseAllocatedPower;
};

struct lldppb_PortManagementAddressInfo {
  char *Ip;
  enum lldppb_ManagementAddressIfSubTypeTypeOptions Subtype;
  char *ID;
};

struct lldppb_ManagementAddress {
  // default: empty, length: 0-128
  char *MACAddr;
  unsigned int Ip_Len; // auto-gen: for list
  char **Ip;
  // default: empty
  char *PortDsr;
};

struct lldppb_ChassisInfo {
  enum lldppb_ChassisIdSubtypeTypeOptions Type;
  char *ID;
};

struct lldppb_PortID {
  enum lldppb_PortIdSubtypeTypeOptions Subtype;
  int32_t VlanID;
  char *ID;
};

struct lldppb_Statistic {
  unsigned int List_Len; // auto-gen: for list
  struct lldppb_StatisticEntry **List;
};

struct lldppb_StatisticEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  int32_t FramesOut;
  int32_t FramesIn;
  int32_t FramesInErrors;
};

struct lldppb_VoiceVlanEntry {
  enum lldppb_PoliciesApplicationTypeOptions ApplicationType;
  bool PolicyDefined;
  bool TaggedVlan;
  int32_t VlanID;
  enum lldppb_PoliciesLayer2PriorityTypeOptions Layer2Priority;
  int32_t DSCP;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_lldp_lldp
