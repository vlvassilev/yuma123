
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
  unsigned long long int InGoodOctets;
  unsigned long long int InBadOctets;
  unsigned long long int InTotalPackets;
  unsigned long long int InUnicasts;
  unsigned long long int InNonUnicasts;
  unsigned long long int InBroadcasts;
  unsigned long long int InMulticasts;
  unsigned long long int InPause;
  unsigned long long int InTotalReceiveErrors;
  unsigned long long int InUndersize;
  unsigned long long int InOversize;
  unsigned long long int InFragments;
  unsigned long long int InJabber;
  unsigned long long int InFcsErrors;
  unsigned long long int InDiscarded;
};

struct rmonpb_Ingress {
  // boundary is according to device.GetPortLists()
  unsigned int List_Len; // auto-gen: for list
  struct rmonpb_IngressEntry **List;
};

struct rmonpb_EgressEntry {
  // Only support INTERFACE_TYPE_PORT
  struct devicepb_InterfaceIdentify *IdentifyNo;
  unsigned long long int OutGoodOctets;
  unsigned long long int OutUnicasts;
  unsigned long long int OutNonUnicasts;
  unsigned long long int OutBroadcasts;
  unsigned long long int OutMulticasts;
  unsigned long long int OutPause;
  unsigned long long int OutDeferred;
  unsigned long long int OutTotalCollisions;
  unsigned long long int OutTotalPackets;
  unsigned long long int OutExcessiveCollisions;
  unsigned long long int OutLateCollisions;
  unsigned long long int OutFcsErrors;
  unsigned long long int OutDroppedPackets;
  unsigned long long int OutMultipleCollisions;
};

struct rmonpb_Egress {
  // boundary is according to device.GetPortLists()
  unsigned int List_Len; // auto-gen: for list
  struct rmonpb_EgressEntry **List;
};

struct rmonpb_HistogramEntry {
  // Only support INTERFACE_TYPE_PORT
  struct devicepb_InterfaceIdentify *IdentifyNo;
  unsigned long long int In64Octets;
  unsigned long long int In65To127Octets;
  unsigned long long int In128To255Octets;
  unsigned long long int In256To511Octets;
  unsigned long long int In512To1023Octets;
  unsigned long long int In1024ToMaxOctets;
};

struct rmonpb_Histogram {
  // boundary is according to device.GetPortLists()
  unsigned int List_Len; // auto-gen: for list
  struct rmonpb_HistogramEntry **List;
};

struct rmonpb_UtilizationRate {
  enum rmonpb_UtilizationIntervalTypeOptions Type;
  long int Rate;
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
