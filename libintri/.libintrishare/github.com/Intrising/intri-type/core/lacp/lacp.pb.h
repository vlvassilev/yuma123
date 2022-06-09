
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_lacp_lacp
#define _H_intri_pb_github_com_Intrising_intri_type_core_lacp_lacp

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

enum lacppb_ConfigLinkAggregationTypeOptions {
  // [Static]
  lacppb_ConfigLinkAggregationTypeOptions_CONFIG_LINK_AGGREGATION_STATIC = 0,
  // [Dyanmic]
  lacppb_ConfigLinkAggregationTypeOptions_CONFIG_LINK_AGGREGATION_DYNAMIC = 1,
};

enum lacppb_ModeTypeOptions {
  // [Passive]
  lacppb_ModeTypeOptions_MODE_TYPE_PASSIVE = 0,
  // [Active]
  lacppb_ModeTypeOptions_MODE_TYPE_ACTIVE = 1,
};

enum lacppb_IntervalTypeOptions {
  // [Slow]
  lacppb_IntervalTypeOptions_INTERVAL_TYPE_SLOW = 0,
  // [Fast]
  lacppb_IntervalTypeOptions_INTERVAL_TYPE_FAST = 1,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct lacppb_Config {
  struct lacppb_SystemConfig *System;
  struct lacppb_LAGConfig *LAG;
};

struct lacppb_SystemConfig {
  // default 32768
  long int SystemPriority;
  enum lacppb_ModeTypeOptions Mode;
  enum lacppb_IntervalTypeOptions TransmitInterval;
};

struct lacppb_LAGConfig {
  // boundary is according to API=device.GetLAGPortLists()
  unsigned int Lists_Len; // auto-gen: for list
  struct lacppb_LAGConfigEntry **Lists;
};

struct lacppb_LAGConfigEntry {
  // Index (for update); unique, generate by device.GetLAGPortLists()
  long int TrunkID;
  enum lacppb_ConfigLinkAggregationTypeOptions LacpEnable;
  // physical port, should not duplicated in list
  struct devicepb_PortList *Identify;
};

struct lacppb_Status {
  unsigned int PortLists_Len; // auto-gen: for list
  struct lacppb_StatusEntry **PortLists;
};

struct lacppb_StatusEntry {
  long int TrunkID;
  struct devicepb_InterfaceIdentify *IdentifyNo;
  enum lacppb_ConfigLinkAggregationTypeOptions LacpEnable;
  struct lacppb_ActorPartnerInfo *Actor;
  struct lacppb_ActorPartnerInfo *Partner;
};

struct lacppb_ActorPartnerInfo {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  long int PortPriority;
  long int SystemPriority;
  unsigned char *MACAddress;
  long int AdminKey;
  long int OperKey;
  struct lacppb_State *Status;
};

struct lacppb_State {
  bool Activity;
  bool Timeout;
  bool Aggr;
  bool Sync;
  bool Collect;
  bool Distribute;
  bool Defaulted;
  bool Expired;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_lacp_lacp
