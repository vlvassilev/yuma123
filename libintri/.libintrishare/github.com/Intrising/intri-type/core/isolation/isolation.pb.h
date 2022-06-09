
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_isolation_isolation
#define _H_intri_pb_github_com_Intrising_intri_type_core_isolation_isolation

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

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

// Only support INTERFACE_TYPE_PORT, INTERFACE_TYPE_PORT
struct isolationpb_ConfigEntry {
  // The primary key of this entry
  struct devicepb_InterfaceIdentify *IdentifyNo;
  // The allow_outgoing_list can't not insert the port of the primary key
  unsigned int AllowOutgoingList_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **AllowOutgoingList;
};

struct isolationpb_Config {
  // boundary is according to device.GetPortLists()
  unsigned int List_Len; // auto-gen: for list
  struct isolationpb_ConfigEntry **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_isolation_isolation
