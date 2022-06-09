
#ifndef _H_intri_pb_google_golang_org_genproto_googleapis_type_timeofday_timeofday
#define _H_intri_pb_google_golang_org_genproto_googleapis_type_timeofday_timeofday

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
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

// Represents a time of day. The date and time zone are either not significant
// or are specified elsewhere. An API may choose to allow leap seconds. Related
// types are [google.type.Date][google.type.Date] and
// `google.protobuf.Timestamp`.
struct timeofdaypb_TimeOfDay {
  // Hours of day in 24 hour format. Should be from 0 to 23. An API may choose
// to allow the value "24:00:00" for scenarios like business closing time.
  long int Hours;
  // Minutes of hour of day. Must be from 0 to 59.
  long int Minutes;
  // Seconds of minutes of the time. Must normally be from 0 to 59. An API may
// allow the value 60 if it allows leap-seconds.
  long int Seconds;
  // Fractions of seconds in nanoseconds. Must be from 0 to 999,999,999.
  long int Nanos;
};
#endif // _H_intri_pb_google_golang_org_genproto_googleapis_type_timeofday_timeofday
