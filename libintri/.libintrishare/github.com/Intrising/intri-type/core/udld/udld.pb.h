
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_udld_udld
#define _H_intri_pb_github_com_Intrising_intri_type_core_udld_udld

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum udldpb_ModeTypeOptions {
  // [Normal] UDLD detects the unidirectional link, the state will change to `undetermined` and send event.
  udldpb_ModeTypeOptions_MODE_TYPE_NORMAL = 0,
  // [Aggressive] UDLD detects the unidirectional link, and sends the `UDLD probe message` for `N` seconds continually, if has no receives the `UDLD echo message`, the state will change to `err-disable` to drop all packets.
  udldpb_ModeTypeOptions_MODE_TYPE_AGGRESSIVE = 1,
};

enum udldpb_PortStateTypeOptions {
  // [None] the UDLD is not enabled.
  udldpb_PortStateTypeOptions_PORT_STATE_TYPE_NONE = 0,
  // [Detection] the UDLD is detecting the link.
  udldpb_PortStateTypeOptions_PORT_STATE_TYPE_DETECTION = 1,
  // [Bidirectionality] the UDLD detects bidirectional link.
  udldpb_PortStateTypeOptions_PORT_STATE_TYPE_BIDIRECTIONALITY = 2,
  // [Undetermined] the UDLD detects unidirectional link, will send event.
  udldpb_PortStateTypeOptions_PORT_STATE_TYPE_UNDETERMINED = 3,
  // [Error-Disabled] the UDLD detects unidirectional link, will drop all packet.
  udldpb_PortStateTypeOptions_PORT_STATE_TYPE_ERROR_DISABLED = 4,
};

enum udldpb_PortFaultTypeOptions {
  // [None] failed get state
  udldpb_PortFaultTypeOptions_PORT_FAULT_TYPE_NONE = 0,
  // [Tx] the port is tx only
  udldpb_PortFaultTypeOptions_PORT_FAULT_TYPE_TX = 1,
  // [Rx] the port is rx only
  udldpb_PortFaultTypeOptions_PORT_FAULT_TYPE_RX = 2,
  // [Both] the port is tx+rx
  udldpb_PortFaultTypeOptions_PORT_FAULT_TYPE_BOTH = 3,
};

enum udldpb_PacketOpcodeTypeOptions {
  // [Reserved] Reserved, the hex is 0x00
  udldpb_PacketOpcodeTypeOptions_PACKET_OPCODE_TYPE_OPTIONS_RESERVED = 0,
  // [Probe] Probe message, the hex is 0x01
  udldpb_PacketOpcodeTypeOptions_PACKET_OPCODE_TYPE_OPTIONS_PROBE = 1,
  // [Echo] Echo message, the hex is 0x02
  udldpb_PacketOpcodeTypeOptions_PACKET_OPCODE_TYPE_OPTIONS_ECHO = 2,
  // [Flush] Flush message, the hex is 0x03
  udldpb_PacketOpcodeTypeOptions_PACKET_OPCODE_TYPE_OPTIONS_FLUSH = 3,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct udldpb_Config {
  struct udldpb_BasicConfig *BasicConfig;
  struct udldpb_PortConfig *PortConfig;
};

struct udldpb_BasicConfig {
  bool IsEnabled;
  enum udldpb_ModeTypeOptions ModeOption;
  long int MessageIntervalSeconds;
};

struct udldpb_PortConfig {
  unsigned int List_Len; // auto-gen: for list
  struct udldpb_PortConfigEntry **List;
};

struct udldpb_PortConfigEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool IsEnabled;
  enum udldpb_ModeTypeOptions ModeOption;
};

struct udldpb_Status {
  struct udldpb_PortStatus *PortStatus;
  struct udldpb_NeighborStatus *NeighborStatus;
};

struct udldpb_PortStatus {
  unsigned int List_Len; // auto-gen: for list
  struct udldpb_PortStatusEntry **List;
};

struct udldpb_PortStatusEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool IsEnabled;
  bool PortLinkUp;
  enum udldpb_PortFaultTypeOptions FaultOption;
  enum udldpb_PortStateTypeOptions StateOption;
  enum udldpb_ModeTypeOptions ModeOption;
};

struct udldpb_NeighborStatus {
  unsigned int List_Len; // auto-gen: for list
  struct udldpb_NeighborStatusEntry **List;
};

struct udldpb_NeighborStatusEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  char *DeviceID;
  char *PortID;
  char *MacAddress;
  char *DeviceName;
  enum udldpb_PortStateTypeOptions StateOption;
  long int ExpirationTimeSeconds;
  long int MessageTimeSeconds;
};

struct udldpb_Statistics {
  struct udldpb_PacketStatistics *Total;
  struct udldpb_PortStatistics *Port;
};

struct udldpb_PortStatistics {
  unsigned int List_Len; // auto-gen: for list
  struct udldpb_PortStatisticsEntry **List;
};

struct udldpb_PortStatisticsEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  struct udldpb_PacketStatistics *Statistics;
};

struct udldpb_PacketStatistics {
  unsigned int List_Len; // auto-gen: for list
  struct udldpb_PacketStatisticsEntry **List;
};

struct udldpb_PacketStatisticsEntry {
  enum udldpb_PacketOpcodeTypeOptions OpcodeOption;
  unsigned long long int Count;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_udld_udld
