
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_cdp_cdp
#define _H_intri_pb_github_com_Intrising_intri_type_core_cdp_cdp

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

enum cdppb_VersionTypeOptions {
  // [V1] Use the CDP V1
  cdppb_VersionTypeOptions_VERSION_TYPE_V1 = 0,
  // [V2] Use the CDP V2
  cdppb_VersionTypeOptions_VERSION_TYPE_V2 = 1,
};

enum cdppb_CapabilityTypeOptions {
  // [Router] Show the capability as a router
  cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_ROUTER = 0,
  // [Transparent Bridge] Show the capability as a transparent bridge
  cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_TRANSPARENT_BRIDGE = 1,
  // [Route Bridge] Show the capability as a route bridge
  cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_SOURCE_ROUTE_BRIDGE = 2,
  // [Switch] Show the capability as a switch
  cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_SWITCH = 3,
  // [Host] Show the capability as a host
  cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_HOST = 4,
  // [IGMP] Show the capability as an IGMP capable device
  cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_IGMP_CAPABLE = 5,
  // [Repeater] Show the capability as a repeater
  cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_REPEATER = 6,
  // [VoIP Phone] Show the capability as a VoIP Phone
  cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_VOIP_PHONE = 7,
  // [Remotely Managed Device] Show the capability as a remotely managed device
  cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_REMOTELY_MANAGED_DEVICE = 8,
  // [Camara] Show the capability as a camera
  cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_CVTA_STP_DISPUTE_RESOLUTION_CISCO_VT_CAMERA = 9,
  // [Two Port MAC Replay] Show the capability as a two port MAC reply
  cdppb_CapabilityTypeOptions_CAPABILITY_TYPE_TWO_PORT_MAC_REPLY = 10,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct cdppb_Config {
  struct cdppb_SystemConfig *SystemConfig;
};

struct cdppb_SystemConfig {
  // default: false
  bool Enabled;
  // default: 180, The unit of this field is second
  int32_t TimeToLive;
  // default: 10, The unit of this field is second
  int32_t MsgTxInterval;
  // enum to str for core configuartion
  enum cdppb_VersionTypeOptions CDPVersion;
};

struct cdppb_LocalInfo {
  unsigned int List_Len; // auto-gen: for list
  struct cdppb_LocalInfoEntry **List;
};

struct cdppb_LocalInfoEntry {
  //  only support Physical port
  struct devicepb_InterfaceIdentify *IdentifyNo;
  struct cdppb_PowerAvailable *PowerAvailable;
  struct cdppb_SystemInfo *SystemInfo;
  struct cdppb_VoiceVLAN *VoiceVlan;
  struct cdppb_PortInfo *PortInfo;
  unsigned int Capability_Len; // auto-gen: for list
  enum cdppb_CapabilityTypeOptions *Capability;
};

struct cdppb_NeighborInfo {
  unsigned int List_Len; // auto-gen: for list
  struct cdppb_NeighborInfoEntry **List;
};

struct cdppb_NeighborInfoEntry {
  struct cdppb_RecvIdentifyInfo *IdentifyNo;
  struct cdppb_SystemInfo *SystemInfo;
  struct cdppb_PowerAvailable *PowerAvailable;
  int64_t TTL;
  int32_t VoIPVlan;
};

struct cdppb_RecvIdentifyInfo {
  //  only support Physical port
  struct devicepb_InterfaceIdentify *RecvIdentifyNo;
  char *PortThroughInterface;
  unsigned int Capability_Len; // auto-gen: for list
  enum cdppb_CapabilityTypeOptions *Capability;
};

struct cdppb_SystemInfo {
  unsigned int IP_Len; // auto-gen: for list
  char **IP;
  char *DeviceID;
  char *Platform;
  char *SystemName;
  char *SoftwareVersion;
  enum cdppb_VersionTypeOptions CDPVersion;
};

struct cdppb_VoiceVLAN {
  unsigned char *Data;
  int32_t VoiceVlan;
};

struct cdppb_PortInfo {
  char *PortThroughInterface;
};

struct cdppb_PowerAvailable {
  int32_t RequestID;
  int32_t ManagementID;
  float Allocated;
  float Supported;
};

struct cdppb_NeighborPoe {
  int32_t RequestId;
  float Allocated;
};

struct cdppb_Statistic {
  unsigned int List_Len; // auto-gen: for list
  struct cdppb_StatisticEntry **List;
};

struct cdppb_StatisticEntry {
  //  only support Physical port
  struct devicepb_InterfaceIdentify *IdentifyNo;
  uint64_t FramesOutV1;
  uint64_t FramesOutV2;
  uint64_t FramesInV1;
  uint64_t FramesInV2;
  uint64_t IllegalChecksum;
  uint64_t OtherErrors;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_cdp_cdp
