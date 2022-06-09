
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_gvrp_gvrp
#define _H_intri_pb_github_com_Intrising_intri_type_core_gvrp_gvrp

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/common/common.pb.h"
#include "../../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum gvrppb_PortModeTypeOptions {
  // [Normal] this mode can register/deregister static/dynamic VLAN dynamically
  gvrppb_PortModeTypeOptions_PORT_MODE_NORMAL = 0,
  // [Fixed] this mode cannot register/deregister VLAN dynamically, only share static VLAN information
  gvrppb_PortModeTypeOptions_PORT_MODE_FIXED = 1,
  // [Forbidden] this mode cannot register/deregister VLAN dynamically, onle share default VLAN information
  gvrppb_PortModeTypeOptions_PORT_MODE_FORBIDDEN = 2,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct gvrppb_PortConfigEntry {
  // Index (for update); physical port and lag port
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool IsEnabled;
  enum gvrppb_PortModeTypeOptions Mode;
  // unit: ms
  long int LeaveAllTimerInterval;
  // unit: ms, the join timer time upper threshold should less than half of leave timer time
  long int JoinTimerInterval;
  // unit: ms, the hold timer time upper threshold should less than half of join timer time
  long int HoldTimerInterval;
  // unit: ms, the leave timer time upper threshold should less than leave all timer time
  long int LeaveTimerInterval;
};

struct gvrppb_PortsConfig {
  unsigned int List_Len; // auto-gen: for list
  struct gvrppb_PortConfigEntry **List;
};

struct gvrppb_Config {
  bool IsEnabled;
  struct gvrppb_PortsConfig *PortsConfig;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_gvrp_gvrp
