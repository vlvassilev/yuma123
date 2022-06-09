
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_sfp_sfp
#define _H_intri_pb_github_com_Intrising_intri_type_core_sfp_sfp

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/event/event.pb.h"
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

struct sfppb_Config {
  // If true , an event is sent when a signal loss occurs
  bool LossOfSignalEvent;
};

struct sfppb_Info {
  unsigned int List_Len; // auto-gen: for list
  struct eventpb_SFPInfo **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_sfp_sfp
