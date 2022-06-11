
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_stormcontrol_stormcontrol
#define _H_intri_pb_github_com_Intrising_intri_type_core_stormcontrol_stormcontrol

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/common/common.pb.h"
#include "../../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>
#include <stdint.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum stormcontrolpb_StormControlActionTypeOptions {
  // [Shutdown] the port interface will be shutdown when the percent of the outputed packet is over the max level
  stormcontrolpb_StormControlActionTypeOptions_STORM_CONTROL_ACTION_TYPE_SHUTDOWN = 0,
  // [Blocking] the port interface will be blocking when the percent of the outputed packet is over the max level
  stormcontrolpb_StormControlActionTypeOptions_STORM_CONTROL_ACTION_TYPE_BLOCKING = 1,
};

enum stormcontrolpb_StormControlSuppressionTypeOptions {
  // [Broadcast] counter forwards broadcast bits/packets per second of lading as the standard
  stormcontrolpb_StormControlSuppressionTypeOptions_STORM_CONTROL_SUPPRESSION_TYPE_BROADCAST = 0,
  // [Multicast] counter forwards multicast bits/packets per second of lading as the standard
  stormcontrolpb_StormControlSuppressionTypeOptions_STORM_CONTROL_SUPPRESSION_TYPE_MULTICAST = 1,
  // [Unknown Unicast] counter forwards unknown unicast bits/packets per second of lading as the standard
  stormcontrolpb_StormControlSuppressionTypeOptions_STORM_CONTROL_SUPPRESSION_TYPE_UNKNOWN_UNICAST = 2,
};

enum stormcontrolpb_StormControlSuppressionUnitTypeOptions {
  // [bits/sec.] counter bits per secound
  stormcontrolpb_StormControlSuppressionUnitTypeOptions_STORM_CONTROL_SUPPRESSION_TYPE_BITS_PER_SECOND = 0,
  // [packets/sec.] counter packets per secound
  stormcontrolpb_StormControlSuppressionUnitTypeOptions_STORM_CONTROL_SUPPRESSION_TYPE_PACKETS_PER_SECOND = 1,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct stormcontrolpb_SuppressionConfigEntry {
  // index (for update), auto generate by StormControlSuppressionTypeOptions
  enum stormcontrolpb_StormControlSuppressionTypeOptions SuppressionOption;
  bool IsEnable;
  enum stormcontrolpb_StormControlActionTypeOptions ActionOption;
  enum stormcontrolpb_StormControlSuppressionUnitTypeOptions UnitOption;
  uint64_t BpsUpperThreshold;
  uint64_t PpsUpperThreshold;
};

struct stormcontrolpb_PortConfigEntry {
  // Index (for update); physcial port
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool IsEnable;
  unsigned int SuppressionList_Len; // auto-gen: for list
  struct stormcontrolpb_SuppressionConfigEntry **SuppressionList;
};

struct stormcontrolpb_PortConfig {
  unsigned int List_Len; // auto-gen: for list
  struct stormcontrolpb_PortConfigEntry **List;
};

struct stormcontrolpb_Config {
  bool IsEnable;
  struct stormcontrolpb_PortConfig *PortConfig;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_stormcontrol_stormcontrol
