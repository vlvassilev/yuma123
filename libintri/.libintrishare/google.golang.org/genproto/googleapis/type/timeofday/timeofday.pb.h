
#ifndef _H_intri_pb_google_golang_org_genproto_googleapis_type_timeofday_timeofday
#define _H_intri_pb_google_golang_org_genproto_googleapis_type_timeofday_timeofday

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
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

// Represents a time of day. The date and time zone are either not significant
// or are specified elsewhere. An API may choose to allow leap seconds. Related
// types are [google.type.Date][google.type.Date] and
// `google.protobuf.Timestamp`.
struct timeofdaypb_TimeOfDay {
  // Hours of day in 24 hour format. Should be from 0 to 23. An API may choose
// to allow the value "24:00:00" for scenarios like business closing time.
  int32_t Hours;
  // Minutes of hour of day. Must be from 0 to 59.
  int32_t Minutes;
  // Seconds of minutes of the time. Must normally be from 0 to 59. An API may
// allow the value 60 if it allows leap-seconds.
  int32_t Seconds;
  // Fractions of seconds in nanoseconds. Must be from 0 to 999,999,999.
  int32_t Nanos;
};
#endif // _H_intri_pb_google_golang_org_genproto_googleapis_type_timeofday_timeofday
