
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_loop_loop
#define _H_intri_pb_github_com_Intrising_intri_type_core_loop_loop

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
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

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct looppb_Config {
  unsigned int PortList_Len; // auto-gen: for list
  struct looppb_PortConfigEntry **PortList;
};

struct looppb_PortConfigEntry {
  // Index (for update); physical port
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool Enabled;
};

struct looppb_PortConfig {
  unsigned int List_Len; // auto-gen: for list
  struct looppb_PortConfigEntry **List;
};

struct looppb_StatusEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  struct timestamppb_Timestamp *LastLoopTs;
  int32_t LoopCount;
  bool IsLoopCurrent;
};

struct looppb_Status {
  unsigned int List_Len; // auto-gen: for list
  struct looppb_StatusEntry **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_loop_loop
