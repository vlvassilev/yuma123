
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_rmon_rmon
#define _H_intri_pb_github_com_Intrising_intri_type_core_rmon_rmon

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

enum rmonpb_UtilizationIntervalTypeOptions {
  // [Now]
  rmonpb_UtilizationIntervalTypeOptions_UTILIZATION_INTERVAL_TYPE_NOW = 0,
  // [30s]
  rmonpb_UtilizationIntervalTypeOptions_UTILIZATION_INTERVAL_TYPE_30_SECONDS = 1,
  // [5min]
  rmonpb_UtilizationIntervalTypeOptions_UTILIZATION_INTERVAL_TYPE_5_MINUTES = 2,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct rmonpb_IngressEntry {
  // Only support INTERFACE_TYPE_PORT
  struct devicepb_InterfaceIdentify *IdentifyNo;
  uint64_t InGoodOctets;
  uint64_t InBadOctets;
  uint64_t InTotalPackets;
  uint64_t InUnicasts;
  uint64_t InNonUnicasts;
  uint64_t InBroadcasts;
  uint64_t InMulticasts;
  uint64_t InPause;
  uint64_t InTotalReceiveErrors;
  uint64_t InUndersize;
  uint64_t InOversize;
  uint64_t InFragments;
  uint64_t InJabber;
  uint64_t InFcsErrors;
  uint64_t InDiscarded;
};

struct rmonpb_Ingress {
  // boundary is according to device.GetPortLists()
  unsigned int List_Len; // auto-gen: for list
  struct rmonpb_IngressEntry **List;
};

struct rmonpb_EgressEntry {
  // Only support INTERFACE_TYPE_PORT
  struct devicepb_InterfaceIdentify *IdentifyNo;
  uint64_t OutGoodOctets;
  uint64_t OutUnicasts;
  uint64_t OutNonUnicasts;
  uint64_t OutBroadcasts;
  uint64_t OutMulticasts;
  uint64_t OutPause;
  uint64_t OutDeferred;
  uint64_t OutTotalCollisions;
  uint64_t OutTotalPackets;
  uint64_t OutExcessiveCollisions;
  uint64_t OutLateCollisions;
  uint64_t OutFcsErrors;
  uint64_t OutDroppedPackets;
  uint64_t OutMultipleCollisions;
};

struct rmonpb_Egress {
  // boundary is according to device.GetPortLists()
  unsigned int List_Len; // auto-gen: for list
  struct rmonpb_EgressEntry **List;
};

struct rmonpb_HistogramEntry {
  // Only support INTERFACE_TYPE_PORT
  struct devicepb_InterfaceIdentify *IdentifyNo;
  uint64_t In_64Octets;
  uint64_t In_65To_127Octets;
  uint64_t In_128To_255Octets;
  uint64_t In_256To_511Octets;
  uint64_t In_512To_1023Octets;
  uint64_t In_1024ToMaxOctets;
};

struct rmonpb_Histogram {
  // boundary is according to device.GetPortLists()
  unsigned int List_Len; // auto-gen: for list
  struct rmonpb_HistogramEntry **List;
};

struct rmonpb_UtilizationRate {
  enum rmonpb_UtilizationIntervalTypeOptions Type;
  int32_t Rate;
};

struct rmonpb_UtilizationEntry {
  // Only support INTERFACE_TYPE_PORT
  struct devicepb_InterfaceIdentify *IdentifyNo;
  unsigned int Ingress_Len; // auto-gen: for list
  struct rmonpb_UtilizationRate **Ingress;
  unsigned int Egress_Len; // auto-gen: for list
  struct rmonpb_UtilizationRate **Egress;
};

struct rmonpb_Utilization {
  // boundary is according to device.GetPortLists()
  unsigned int List_Len; // auto-gen: for list
  struct rmonpb_UtilizationEntry **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_rmon_rmon
