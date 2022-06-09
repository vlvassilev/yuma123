
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_diagnostic_diagnostic
#define _H_intri_pb_github_com_Intrising_intri_type_core_diagnostic_diagnostic

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/common/common.pb.h"
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

struct diagnosticpb_ARPEntry {
  char *IPAddr;
  char *MACAddr;
};

struct diagnosticpb_ARPTables {
  unsigned int List_Len; // auto-gen: for list
  struct diagnosticpb_ARPEntry **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_diagnostic_diagnostic
