
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_system_system
#define _H_intri_pb_github_com_Intrising_intri_type_core_system_system

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
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

struct systempb_Status {
  char *LastBootTime;
  long long int Uptime;
};

struct systempb_Config {
  char *SysName;
  char *SysLocation;
  char *SysGroup;
  char *SysContact;
};

struct systempb_IdentificationConfig {
  char *SysName;
  char *SysLocation;
  char *SysGroup;
  char *SysContact;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_system_system
