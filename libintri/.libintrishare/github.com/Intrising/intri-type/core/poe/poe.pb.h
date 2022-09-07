
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_poe_poe
#define _H_intri_pb_github_com_Intrising_intri_type_core_poe_poe

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../../github.com/Intrising/intri-type/hardware/hardware.pb.h"
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

enum poepb_LLDPDeliverUsageTypeOptions {
  poepb_LLDPDeliverUsageTypeOptions_LLDP_L2_USAGE_TYPE_L1 = 0,
  poepb_LLDPDeliverUsageTypeOptions_LLDP_L2_USAGE_TYPE_L1_AUTO = 1,
  poepb_LLDPDeliverUsageTypeOptions_LLDP_L2_USAGE_TYPE_LLDP = 2,
  poepb_LLDPDeliverUsageTypeOptions_LLDP_L2_USAGE_TYPE_LLDP_AUTO = 3,
  poepb_LLDPDeliverUsageTypeOptions_LLDP_L2_USAGE_TYPE_CDP = 4,
  poepb_LLDPDeliverUsageTypeOptions_LLDP_L2_USAGE_TYPE_CDP_AUTO = 5,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct poepb_Config {
  struct poepb_SystemBudget *Budget;
  struct poepb_PortConfig *Port;
};

struct poepb_SystemBudget {
  int32_t MaxPowerAvailable;
};

struct poepb_PortConfig {
  unsigned int List_Len; // auto-gen: for list
  struct poepb_ConfigEntry **List;
};

struct poepb_ConfigEntry {
  // Index for (update), physical port only
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool Enabled;
  enum hardwarepb_PoEPortModeTypeOptions Mode;
  enum hardwarepb_PoEPriorityLevelTypeOptions Priority;
};

struct poepb_SystemStatus {
  // No show on UI, it's not for general user
  double MinShutDownVoltage;
  // No show on UI, it's not for general user
  double MaxShutDownVoltage;
  double VMainVoltage;
  double IMainCurrent;
  double PowerConsumption;
};

struct poepb_PortStatus {
  unsigned int List_Len; // auto-gen: for list
  struct poepb_PortStatusEntry **List;
};

struct poepb_PortStatusEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  // PoE status and conditions.
  enum hardwarepb_PoEConditionTypeOptions Condition;
  // Determined and negotiated PoE class.
  enum hardwarepb_PoEDeterminedClassTypeOptions DeterminedClass;
  enum hardwarepb_PoEPriorityLevelTypeOptions Priority;
  // Current delivered to the attached device. (units: A)
  float OutputCurrent;
  // Voltage delivered to the attached device. (units: V)
  float OutputVoltage;
  // Calculated power delivered to the attached device. (units: W)
  float OutputPower;
  float SupportMaxPower;
};

struct poepb_SetPoEMaxPowerAvailableRequest {
  // (units: W)
  int32_t MaxPowerAvailable;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_poe_poe
