
#ifndef _H_intri_pb_github_com_Intrising_intri_type_log_log
#define _H_intri_pb_github_com_Intrising_intri_type_log_log

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../github.com/Intrising/intri-type/common/common.pb.h"
#include "../../../../github.com/Intrising/intri-type/event/event.pb.h"
#include "../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include "../../../../github.com/golang/protobuf/ptypes/timestamp/timestamp.pb.h"
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

enum logpb_StorageTypeOptions {
  // [Flash] Log file is stored in flash
  logpb_StorageTypeOptions_STORAGE_TYPE_FLASH = 0,
  // [Ram Disk] Log file is stored in ram disk
  logpb_StorageTypeOptions_STORAGE_TYPE_RAM_DISK = 1,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct logpb_Config {
  struct logpb_BasicConfig *BasicConfig;
  struct logpb_TargetConfig *TargetConfig;
  struct logpb_ActionConfig *ActionConfig;
};

struct logpb_BasicConfig {
  enum logpb_StorageTypeOptions StorageOption;
};

struct logpb_TargetConfig {
  unsigned int List_Len; // auto-gen: for list
  struct logpb_TargetConfigEntry **List;
};

struct logpb_TargetConfigEntry {
  // Index (for add/delete); unique, 
// boundary is according to BoundaryLog.log_target_list
  char *Name;
  char *HostAddress;
  enum eventpb_TargetLogTypeOptions TargetLogType;
  enum eventpb_LoggingSeverityTypeOptions LoggingSeverityType;
  bool LogConfigChanges;
  bool LogDebugEventOnly;
  char *SnmpV3Username;
  char *SnmpTrapCommunity;
};

struct logpb_ActionConfig {
  unsigned int List_Len; // auto-gen: for list
  struct logpb_ActionConfigEntry **List;
};

struct logpb_ActionConfigEntry {
  enum eventpb_LoggingTypeOptions LoggingType;
  enum eventpb_LoggingSeverityTypeOptions LoggingSeverityType;
};

struct logpb_Statistics {
  uint64_t NumberOfTargets;
  int32_t SyslogCounter;
  int32_t SnmpTrapCounter;
  int32_t SnmpInfoCounter;
  int32_t DisplayInCliCounter;
};

struct logpb_LogFileEntry {
  uint64_t LogID;
  struct timestamppb_Timestamp *Ts;
  enum eventpb_LoggingTypeOptions LoggingType;
  enum eventpb_LoggingSeverityTypeOptions LoggingSeverityType;
  char *Message;
};

struct logpb_LogFiles {
  unsigned int List_Len; // auto-gen: for list
  struct logpb_LogFileEntry **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_log_log
