
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_timerange_timerange
#define _H_intri_pb_github_com_Intrising_intri_type_core_timerange_timerange

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/common/common.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include "../../../../../google.golang.org/genproto/googleapis/type/date/date.pb.h"
#include "../../../../../google.golang.org/genproto/googleapis/type/dayofweek/dayofweek.pb.h"
#include "../../../../../google.golang.org/genproto/googleapis/type/timeofday/timeofday.pb.h"
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

enum timerangepb_CommandTypeOptions {
  // [Periodic]
  timerangepb_CommandTypeOptions_COMMAND_TYPE_PERIODIC = 0,
  // [Absolute]
  timerangepb_CommandTypeOptions_COMMAND_TYPE_ABSOLUTE = 1,
};

enum timerangepb_PeriodicDayTypeOptions {
  // [Day of the week]
  timerangepb_PeriodicDayTypeOptions_PERIODIC_DAY_TYPE_DAYOFWEEK = 0,
  // [Daily]
  timerangepb_PeriodicDayTypeOptions_PERIODIC_DAY_TYPE_DAILY = 1,
  // [Weekdays]
  timerangepb_PeriodicDayTypeOptions_PERIODIC_DAY_TYPE_WEEKDAYS = 2,
  // [Weekend]
  timerangepb_PeriodicDayTypeOptions_PERIODIC_DAY_TYPE_WEEKEND = 3,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct timerangepb_Config {
  // boundary is according to device.GetBoundary().TimeRange.EntryRange
  unsigned int List_Len; // auto-gen: for list
  struct timerangepb_ConfigEntry **List;
};

struct timerangepb_ConfigEntry {
  // Index (for update / delelte); unique
  char *Name;
  // is this rule enabled
  bool Enabled;
  // select rule type:
// 1. when the enum is 'ABSOLUTE'
//   - fill the field (`start_date` and `start_time`) or (`start_date` , `start_time` , `end_date` and `end_time`)
// 2. when the enum is 'PERIODIC'
//   - fill the field `periodic_day` , `start_time` and `end_time`
  enum timerangepb_CommandTypeOptions Command;
  // PERIODIC
  struct timerangepb_DayTime *PeriodicDay;
  // ABSOLUTE , PERIODIC : the time's accuracy is minute
  struct timeofdaypb_TimeOfDay *StartTime;
  struct timeofdaypb_TimeOfDay *EndTime;
  // ABSOLUTE : if you only fill the start_date, it will take effect forever when the current time has reached start_date
  struct datepb_Date *StartDate;
  struct datepb_Date *EndDate;
};

struct timerangepb_DayTime {
  enum timerangepb_PeriodicDayTypeOptions PeriodicType;
  // only for PERIODIC_DAY_TYPE_DAYOFWEEK
// the enum 'google.type.DayOfWeek' in this list should not be duplicated
  unsigned int DayOfWeekLists_Len; // auto-gen: for list
  enum dayofweekpb_DayOfWeek *DayOfWeekLists;
};

struct timerangepb_EntryStatus {
  // Index from TimeRangeEntry
  char *Name;
  // is this time range entry executing
  bool IsActive;
};

struct timerangepb_Status {
  unsigned int List_Len; // auto-gen: for list
  struct timerangepb_EntryStatus **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_timerange_timerange
