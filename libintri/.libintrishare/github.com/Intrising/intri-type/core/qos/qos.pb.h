
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_qos_qos
#define _H_intri_pb_github_com_Intrising_intri_type_core_qos_qos

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

enum qospb_ModeTypeOptions {
  // [Disable] this mode has no type of traffic prioritized over another queue
  qospb_ModeTypeOptions_MODE_TYPE_DISABLED = 0,
  // [Basic] this mode allows the type of traffic prioritized with the trust mode to queue
  qospb_ModeTypeOptions_MODE_TYPE_BASIC = 1,
};

enum qospb_TrustModeTypeOptions {
  // [CoS] this mode sets CoS/802.1p to queue prioritized
  qospb_TrustModeTypeOptions_TRUST_MODE_TYPE_COS = 0,
  // [DSCP Only] this mode sets DSCP to queue prioritized
  qospb_TrustModeTypeOptions_TRUST_MODE_TYPE_DSCP_ONLY = 1,
  // [DSCP First] this mode sets DSCP to queue as the first priority, and then CoS to queue as the second
  qospb_TrustModeTypeOptions_TRUST_MODE_TYPE_DSCP_FIRST = 2,
};

enum qospb_PrioritySchemeTypeOptions {
  // [WRR] WRR(Weighted Round Robin) queuing schedules all the queues, which in turn ensures every queue served for a certain time
  qospb_PrioritySchemeTypeOptions_PRIORITY_SCHEME_TYPE_WRR = 0,
  // [SP] SP(Strict Priority) queuing allows queue priority to reduce the response delay when congestion occurs
  qospb_PrioritySchemeTypeOptions_PRIORITY_SCHEME_TYPE_SP = 1,
  // [WRR+SP] WRR+SP queuing
  qospb_PrioritySchemeTypeOptions_PRIORITY_SCHEME_TYPE_WRR_AND_SP = 2,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct qospb_WeightedFairTrafficRatioQueueEntry {
  // Index (for update); the field `queue_no` should validated in boundary (`BoundaryQoS`, `queue_list_range`)
  long int QueueNo;
  long int Priority;
  enum qospb_PrioritySchemeTypeOptions Scheme;
};

struct qospb_MappingCoSDot1PToQueueEntry {
  // Index (for update); the field `cos_no` should validated in boundary (`BoundaryQoS`, `co_s_range`)
  long int CosNo;
  long int QueueNo;
};

struct qospb_MappingDSCPToQueueEntry {
  // Index (for update); the field `d_s_c_p_no` should validated in boundary (`BoundaryQoS`, `d_s_c_p_range`)
  long int DSCPNo;
  // the field `queue_no` should exist in `queue_list` in `Config`
  long int QueueNo;
};

struct qospb_InterfaceConfigEntry {
  // Index (for update); physical port
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool IsEnabled;
  long int EgressBandwidth;
  long int IngressBandwidth;
  bool IngressUnicastEnabled;
  bool IngressMulticastEnabled;
  bool IngressBroadcastEnabled;
};

struct qospb_Config {
  enum qospb_ModeTypeOptions ModeOption;
  enum qospb_TrustModeTypeOptions TrustModeOption;
  unsigned int QueueList_Len; // auto-gen: for list
  struct qospb_WeightedFairTrafficRatioQueueEntry **QueueList;
  unsigned int CoSList_Len; // auto-gen: for list
  struct qospb_MappingCoSDot1PToQueueEntry **CoSList;
  unsigned int DSCPList_Len; // auto-gen: for list
  struct qospb_MappingDSCPToQueueEntry **DSCPList;
  unsigned int InterfaceList_Len; // auto-gen: for list
  struct qospb_InterfaceConfigEntry **InterfaceList;
};

struct qospb_Mode {
  enum qospb_ModeTypeOptions Option;
};

struct qospb_TrustMode {
  enum qospb_TrustModeTypeOptions Option;
};

struct qospb_PriorityScheme {
  enum qospb_PrioritySchemeTypeOptions Option;
};

struct qospb_QueueList {
  unsigned int List_Len; // auto-gen: for list
  struct qospb_WeightedFairTrafficRatioQueueEntry **List;
};

struct qospb_CoSList {
  unsigned int List_Len; // auto-gen: for list
  struct qospb_MappingCoSDot1PToQueueEntry **List;
};

struct qospb_DSCPList {
  unsigned int List_Len; // auto-gen: for list
  struct qospb_MappingDSCPToQueueEntry **List;
};

struct qospb_InterfaceList {
  unsigned int List_Len; // auto-gen: for list
  struct qospb_InterfaceConfigEntry **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_qos_qos
