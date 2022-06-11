
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_ptp_ptp
#define _H_intri_pb_github_com_Intrising_intri_type_core_ptp_ptp

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/common/common.pb.h"
#include "../../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/timestamp/timestamp.pb.h"
#include <stdbool.h>
#include <stdint.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum ptppb_TimeStatusTypeOptions {
  // [Disable]
  ptppb_TimeStatusTypeOptions_TIME_STATUS_TYPE_DISABLE = 0,
  // [Sync]
  ptppb_TimeStatusTypeOptions_TIME_STATUS_TYPE_SYNC = 1,
  // [Not Sync]
  ptppb_TimeStatusTypeOptions_TIME_STATUS_TYPE_NOT_SYNC = 2,
};

enum ptppb_EncapsulationTypeOptions {
  // [L2 Ether] PTP message transmit over ethertnet
  ptppb_EncapsulationTypeOptions_ENCAPSULATION_TYPE_ETHER = 0,
  // [UDP IPv4] PTP message transmit over IPv4 UDP
  ptppb_EncapsulationTypeOptions_ENCAPSULATION_TYPE_UDP_IPV4 = 1,
  // *[UDP IPv6] PTP message transmit over IPv6 UDP
  ptppb_EncapsulationTypeOptions_ENCAPSULATION_TYPE_UDP_IPV6 = 2,
};

enum ptppb_TransmissionTypeOptions {
  // [Unicast] PTP message transmit by Unicast
  ptppb_TransmissionTypeOptions_TRANSMISSION_TYPE_UNICAST = 0,
  // [Multicast] PTP message transmit by Multicast
  ptppb_TransmissionTypeOptions_TRANSMISSION_TYPE_MULTICAST = 1,
};

enum ptppb_ProfileTypeOptions {
  // [IEEE 1588v2] PTP profile for IEEE 1588v2
  ptppb_ProfileTypeOptions_PROFILE_TYPE_IEEE_1588_V2 = 0,
  // *[IEEE 802.1AS] PTP profile for IEEE 802.1AS
  ptppb_ProfileTypeOptions_PROFILE_TYPE_IEEE_802_DOT_1_AS = 1,
  // [ITU G8275.1] PTP profile for ITU G8275.1
  ptppb_ProfileTypeOptions_PROFILE_TYPE_ITU_G8275_DOT_1 = 2,
  // [ITU G8275.2] PTP profile for ITU G8275.2
  ptppb_ProfileTypeOptions_PROFILE_TYPE_ITU_G8275_DOT_2 = 3,
  // *[IEEE C37.238-2017] PTP profile for IEEE C37.238-2017
  ptppb_ProfileTypeOptions_PROFILE_TYPE_IEEE_C37_DOT_238_DASH_2017 = 4,
  // *[IEC 61850-9-3] PTP profile for IEC 61850-9-3
  ptppb_ProfileTypeOptions_PROFILE_TYPE_IEC_61850_DASH_9_DASH_3 = 5,
};

enum ptppb_DelayMechanismTypeOptions {
  // [End-to-End] PTP path delay calculation is end-to-end
  ptppb_DelayMechanismTypeOptions_DELAY_MECHANISM_TYPE_E2E = 0,
  // [Peer-to-Peer]  PTP path delay calculation is peer-to-peer
  ptppb_DelayMechanismTypeOptions_DELAY_MECHANISM_TYPE_P2P = 1,
};

enum ptppb_ClockTypeOptions {
  // [Boundary Clock] PTP clock type is Boundary Clock
  ptppb_ClockTypeOptions_CLOCK_TYPE_BC = 0,
  // [Ordinary Clock] PTP clock type is Ordinary Clock
  ptppb_ClockTypeOptions_CLOCK_TYPE_OC = 1,
  // [Transparent Clock] PTP clock type is Transparent Clock
  ptppb_ClockTypeOptions_CLOCK_TYPE_TC = 2,
  // *[Bridge] PTP clock type is Time Aware Bridge
  ptppb_ClockTypeOptions_CLOCK_TYPE_BRIDGE = 3,
};

enum ptppb_ClockAccuracyTypeOptions {
  // *[reserved] reserved
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_RESERVED_00_1F = 0,
  // [within 25ns] The time is accurate to within 25 ns
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_25_NS = 32,
  // [within 100ns] The time is accurate to within 100 ns
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_100_NS = 33,
  // [within 250ns] The time is accurate to within 250 ns
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_250_NS = 34,
  // [within 1us] The time is accurate to within 250 us
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_1_US = 35,
  // [within 2.5us] The time is accurate to within 2.5 us
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_2_DOT_5_US = 36,
  // [within 10us] The time is accurate to within 10 us
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_10_US = 37,
  // [within 25us] The time is accurate to within 25 us
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_25_US = 38,
  // [within 100us] The time is accurate to within 100 us
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_100_US = 39,
  // [within 250us] The time is accurate to within 250 us
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_250_US = 40,
  // [within 1ms] The time is accurate to within 1 ms
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_1_MS = 41,
  // [within 2.5ms] The time is accurate to within 2.5 ms
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_2_DOT_5_MS = 42,
  // [within 10ms] The time is accurate to within 10 ms
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_10_MS = 43,
  // [within 25ms] The time is accurate to within 25 ms
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_25_MS = 44,
  // [within 100ms] The time is accurate to within 100 ms
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_100_MS = 45,
  // [within 250ms] The time is accurate to within 250 ms
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_250_MS = 46,
  // [within 1s] The time is accurate to within 1 s
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_1_S = 47,
  // [within 10s] The time is accurate to within 10 s
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_WITHIN_10_S = 48,
  // [greater than 10s] The time is accurate to greater than 10 s
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_GREATER_THAN_10_S = 49,
  // *[reserved] reserved
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_RESERVED_32_7F = 50,
  // *[reserved] reserved
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_RESERVED_80_FD = 128,
  // [unknown] The time is accurate to unknown
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_UNKNOWN = 254,
  // *[reserved] reserved
  ptppb_ClockAccuracyTypeOptions_CLOCK_ACCURACY_RESERVED_FF = 255,
};

enum ptppb_PortStateTypeOptions {
  // [Initializing] PTP port status is initializing
  ptppb_PortStateTypeOptions_PORT_STATE_INITIALIZING = 0,
  // [Faulty] PTP port status is faulty
  ptppb_PortStateTypeOptions_PORT_STATE_FAULTY = 1,
  // [Disabled] PTP port status is disabled
  ptppb_PortStateTypeOptions_PORT_STATE_DISABLED = 2,
  // [Listening] PTP port status is listening
  ptppb_PortStateTypeOptions_PORT_STATE_LISTENING = 3,
  // [Premaster] PTP port status is premaster
  ptppb_PortStateTypeOptions_PORT_STATE_PRE_MASTER = 4,
  // [Master] PTP port status is master
  ptppb_PortStateTypeOptions_PORT_STATE_MASTER = 5,
  // [Passive] PTP port status is passive
  ptppb_PortStateTypeOptions_PORT_STATE_PASSIVE = 6,
  // [Uncalibrated] PTP port status is uncalibrated
  ptppb_PortStateTypeOptions_PORT_STATE_UNCALIBRATED = 7,
  // [Slave] PTP port status is slave
  ptppb_PortStateTypeOptions_PORT_STATE_SLAVE = 8,
};

enum ptppb_TimeSourceTypeOptions {
  // *[reserved] reserved
  ptppb_TimeSourceTypeOptions_TIME_SOURCE_RESERVED = 0,
  // [Atomic Clock] Time source is Atomic Clock
  ptppb_TimeSourceTypeOptions_TIME_SOURCE_ATOMIC_CLOCK = 16,
  // [GPS] Time source is GPS
  ptppb_TimeSourceTypeOptions_TIME_SOURCE_GPS = 32,
  // [Terrestrial Radio] Time source is Terrestrial Radio
  ptppb_TimeSourceTypeOptions_TIME_SOURCE_TERRESTRIAL_RADIO = 48,
  // [PTP] Time source is PTP
  ptppb_TimeSourceTypeOptions_TIME_SOURCE_PTP = 64,
  // [NTP] Time source is NTP
  ptppb_TimeSourceTypeOptions_TIME_SOURCE_NTP = 80,
  // [Hand Set] Time source is Hand Set
  ptppb_TimeSourceTypeOptions_TIME_SOURCE_HAND_SET = 96,
  // [Other] Time source is Other
  ptppb_TimeSourceTypeOptions_TIME_SOURCE_OTHER = 144,
  // [Internal Oscillator] Time source is Internal Oscillator
  ptppb_TimeSourceTypeOptions_TIME_SOURCE_INTERNAL_OSCILLATOR = 160,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct ptppb_Config {
  struct ptppb_SystemConfig *System;
  struct ptppb_PortConfig *Port;
};

struct ptppb_SystemConfig {
  struct ptppb_SystemModeConfig *Mode;
  struct ptppb_SystemTimerConfig *Timer;
  struct ptppb_SystemClockConfig *Clock;
  //
// mode.profile is
// IEEE_1588_V2    : ENCAPSULATION_TYPE_ETHER , ENCAPSULATION_TYPE_UDP_IPV4
// ITU_G8275_DOT_1 : ENCAPSULATION_TYPE_ETHER
// ITU_G8275_DOT_2 : ENCAPSULATION_TYPE_UDP_IPV4
  enum ptppb_EncapsulationTypeOptions Encapsulation;
  //
// mode.profile is
// IEEE_1588_V2    : TRANSMISSION_TYPE_MULTICAST
// ITU_G8275_DOT_1 : TRANSMISSION_TYPE_MULTICAST
// ITU_G8275_DOT_2 : TRANSMISSION_TYPE_UNICAST
  enum ptppb_TransmissionTypeOptions Transmission;
  //
// transmission is
// MULTICAST : not relevant
// UNICAST   : applicable
  unsigned int IPAddressList_Len; // auto-gen: for list
  struct commonpb_IPAddress **IPAddressList;
};

struct ptppb_SystemModeConfig {
  bool Enabled;
  enum ptppb_ProfileTypeOptions Profile;
  enum ptppb_ClockTypeOptions ClockType;
  //
// mode.profile is
// IEEE_1588_V2    : 0-255 , default is 0
// ITU_G8275_DOT_1 : 24-43 , default is 24
// ITU_G8275_DOT_2 : 44-63 , default is 44
  int32_t DomainNumber;
  //
// mode.profile is
// IEEE_1588_V2    : DELAY_MECHANISM_TYPE_E2E , DELAY_MECHANISM_TYPE_P2P
// ITU_G8275_DOT_1 : DELAY_MECHANISM_TYPE_E2E
// ITU_G8275_DOT_2 : DELAY_MECHANISM_TYPE_E2E
  enum ptppb_DelayMechanismTypeOptions DelayMechanism;
};

struct ptppb_SystemTimerConfig {
  //
// mode.profile is
// IEEE_1588_V2    : -6 ~ 7, default is 0
// ITU_G8275_DOT_1 : not relevant
// ITU_G8275_DOT_2 : not relevant
  int32_t PdelayReq;
  //
// mode.profile is
// IEEE_1588_V2    : 0 ~ 4, default is 1
// ITU_G8275_DOT_1 : 3
// ITU_G8275_DOT_2 : 3
  int32_t Announce;
  //
// mode.profile is
// IEEE_1588_V2    : 2 ~ 10, default is 2
// ITU_G8275_DOT_1 : 3
// ITU_G8275_DOT_2 : 2
  int32_t AnnounceTimeOut;
  //
// mode.profile is
// IEEE_1588_V2    : -1 ~ 1, default is 0
// ITU_G8275_DOT_1 : 0, TODO: standard is 4
// ITU_G8275_DOT_2 : 0, TODO: standard is 6
  int32_t Sync;
  // Default value is 50
  int32_t SyncLimit;
};

struct ptppb_SystemClockConfig {
  //
// mode.profile is
// IEEE_1588_V2    : 0-255, default is 128
// ITU_G8275_DOT_1 : 128
// ITU_G8275_DOT_2 : 128
  int32_t Priority1;
  //
// mode.profile is
// IEEE_1588_V2    : 0-255, default is 128
// ITU_G8275_DOT_1 : 0-255, default is 128
// ITU_G8275_DOT_2 : 0-255, default is 128
  int32_t Priority2;
};

struct ptppb_PortConfig {
  unsigned int List_Len; // auto-gen: for list
  struct ptppb_PortConfigEntry **List;
};

struct ptppb_PortConfigEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool Enabled;
  bool MasterOnly;
};

struct ptppb_Status {
  unsigned int PortStatusList_Len; // auto-gen: for list
  struct ptppb_PortStatusEntry **PortStatusList;
  unsigned int PortCounterList_Len; // auto-gen: for list
  struct ptppb_PortCounterEntry **PortCounterList;
};

struct ptppb_ClockStatus {
  unsigned int ParentList_Len; // auto-gen: for list
  struct ptppb_ClockInfo **ParentList;
  struct ptppb_GrandmasterInfo *Grandmaster;
};

struct ptppb_ClockInfo {
  int32_t DomainNumber;
  unsigned char *ClockIdentity;
  unsigned char *PortIdentity;
  int32_t NumberOfPorts;
  int32_t Priority1;
  int32_t Priority2;
};

struct ptppb_GrandmasterInfo {
  struct ptppb_ClockInfo *ClockInfo;
  struct ptppb_ClockQuality *ClockQuality;
};

struct ptppb_ClockQuality {
  int32_t Class;
  enum ptppb_ClockAccuracyTypeOptions Accuracy;
  enum ptppb_TimeSourceTypeOptions TimeSource;
  int32_t OffsetScaledLogVariance;
  int32_t Offset;
};

struct ptppb_PortStatusEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  unsigned char *ClockIdentity;
  unsigned char *PortIdentity;
  enum ptppb_PortStateTypeOptions State;
  int32_t PathDelay;
};

struct ptppb_PortCounterEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  uint64_t RxPdelayReq;
  uint64_t RxPdelayResp;
  uint64_t RxPdelayRespFollow;
  uint64_t TxPdelayReq;
  uint64_t TxPdelayResp;
  uint64_t TxPdelayRespFollow;
};

struct ptppb_TimeStatus {
  enum ptppb_TimeStatusTypeOptions State;
  struct timestamppb_Timestamp *DateTime;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_ptp_ptp
