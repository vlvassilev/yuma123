
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_time_time
#define _H_intri_pb_github_com_Intrising_intri_type_core_time_time

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/common/common.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/timestamp/timestamp.pb.h"
#include <stdbool.h>
#include <stdint.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum timepb_StatusTypeOptions {
  // [Unset]
  timepb_StatusTypeOptions_STATUS_TYPE_UNSET = 0,
  // [Manually Set]
  timepb_StatusTypeOptions_STATUS_TYPE_MANUALLY_SET = 1,
  // [Synchronized]
  timepb_StatusTypeOptions_STATUS_TYPE_SYNCHRONIZED = 2,
  // [Sync Failed]
  timepb_StatusTypeOptions_STATUS_TYPE_SYNC_FAILED = 3,
  // [Day Light Saving Time]
  timepb_StatusTypeOptions_STATUS_TYPE_DAY_LIGHT_SAVING_TIME = 4,
  // [GNSS Set]
  timepb_StatusTypeOptions_STATUS_TYPE_GNSS_SET = 5,
  // [GNSS Failed]
  timepb_StatusTypeOptions_STATUS_TYPE_GNSS_FAILED = 6,
  // [PTP Set]
  timepb_StatusTypeOptions_STATUS_TYPE_PTP_SET = 7,
  // [PTP Failed]
  timepb_StatusTypeOptions_STATUS_TYPE_PTP_FAILED = 8,
};

enum timepb_ModeTypeOptions {
  // [Manual]
  timepb_ModeTypeOptions_MODE_TYPE_MANUAL = 0,
  // [Auto (Use NTP)] Time mode auto requires Internet access and will get the time from the given main NTP server
  timepb_ModeTypeOptions_MODE_TYPE_AUTO = 1,
  // [GNSS]
  timepb_ModeTypeOptions_MODE_TYPE_GNSS = 2,
  // [PTP]
  timepb_ModeTypeOptions_MODE_TYPE_PTP = 3,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct timepb_Config {
  enum timepb_ModeTypeOptions Mode;
  // The main NTP server that system will get time from
  char *MainNTPServer;
  char *BackupNTPServer;
  bool TrustedServerEnabled;
  // The unit of this field is second
  int32_t SyncInterval;
  // Valid format is the entry in ListTimeZones
  char *TimeZone;
  // remove : unused
  char *TimeFormat;
  char *DateFormat;
  // format is "2006-01-02 15:04:05"
  char *Manual;
};

struct timepb_ListTimeZones {
  unsigned int List_Len; // auto-gen: for list
  char **List;
};

struct timepb_Status {
  enum timepb_StatusTypeOptions Status;
  char *LocalTime;
  char *LocalDate;
  char *UsedNTPServer;
};

struct timepb_RequestWithTimestamp {
  struct timestamppb_Timestamp *Ts;
};

struct timepb_RequestWithInt64 {
  // The unit of this field is second
  int64_t Ts;
};

struct timepb_Response {
  char *Ts;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_time_time
