
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_mirroring_mirroring
#define _H_intri_pb_github_com_Intrising_intri_type_core_mirroring_mirroring

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

enum mirroringpb_DestinationTypeOptions {
  // [Local] to set SPAN(Local Switched Port Analyzer) destination port interface
  mirroringpb_DestinationTypeOptions_DESTINATION_TYPE_LOCAL = 0,
  // [Remote] to set RSPAN(Remote SPAN) VLAN
  mirroringpb_DestinationTypeOptions_DESTINATION_TYPE_REMOTE = 1,
};

enum mirroringpb_SourceInterfaceTypeOptions {
  // [Local] to set SPAN(Local Switched Port Analyzer) source port interface
  mirroringpb_SourceInterfaceTypeOptions_SOURCE_INTERFACE_TYPE_LOCAL = 0,
  // *[Internal] to set source port interface
  mirroringpb_SourceInterfaceTypeOptions_SOURCE_INTERFACE_TYPE_VLAN = 1,
  // [Remote] to set RSPAN(Remote SPAN) VLAN
  mirroringpb_SourceInterfaceTypeOptions_SOURCE_INTERFACE_TYPE_REMOTE = 2,
};

enum mirroringpb_DirectionTypeOptions {
  // [None] to disable mirroring direction
  mirroringpb_DirectionTypeOptions_MIRRORING_DIRECTION_TYPE_NONE = 0,
  // [TX only] to set mirroring direction TX only
  mirroringpb_DirectionTypeOptions_MIRRORING_DIRECTION_TYPE_TX_ONLY = 1,
  // [RX only] to set mirroring direction RX only
  mirroringpb_DirectionTypeOptions_MIRRORING_DIRECTION_TYPE_RX_ONLY = 2,
  // [TX+RX] to set mirroring direction TX and RX both
  mirroringpb_DirectionTypeOptions_MIRRORING_DIRECTION_TYPE_BOTH = 3,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

// Config for `Mirror` 
struct mirroringpb_Config {
  struct mirroringpb_RSPANConfig *RSPAN;
  struct mirroringpb_DestinationSession *DestinationList;
  struct mirroringpb_SourceSession *SourceList;
};

struct mirroringpb_DestinationSessionEntry {
  // Index (for add/update/delete); the index should validated in boundary (`BoundaryMirroring`, `session`)
  long int Index;
  enum mirroringpb_DestinationTypeOptions Type;
  // phycial port only, using device_i_d and port_no
  struct devicepb_InterfaceIdentify *IdentifyNo;
};

struct mirroringpb_DestinationSession {
  // the length should validated in boundary (`BoundaryMirroring`, `session`)
  unsigned int List_Len; // auto-gen: for list
  struct mirroringpb_DestinationSessionEntry **List;
};

struct mirroringpb_SourceSessionEntry {
  // Index (for add/update/delete)
// the index should validated in boundary (`BoundaryMirroring`, `session`)
// the index should validated in destination list (`DestinationSession`, `list`)
  long int DestinationIndex;
  enum mirroringpb_SourceInterfaceTypeOptions SourceInterface;
  // only for SOURCE_INTERFACE_TYPE_LOCAL
// phycial port, the type INTERFACE_TYPE_PORT only, using device_i_d and port_no
  struct devicepb_InterfaceIdentify *IdentifyNo;
  // when using SOURCE_INTERFACE_TYPE_REMOTE direction_type need to be MIRRORING_DIRECTION_TYPE_RX_ONLY
  enum mirroringpb_DirectionTypeOptions DirectionType;
};

struct mirroringpb_SourceSession {
  unsigned int List_Len; // auto-gen: for list
  struct mirroringpb_SourceSessionEntry **List;
};

struct mirroringpb_RSPANConfig {
  bool Enabled;
  // the field `vlan_i_d` should exist in VLAN filter list (`VLANConfig`, `filters`)
  long int VlanID;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_mirroring_mirroring
