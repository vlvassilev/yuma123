
#ifndef _H_intri_pb_github_com_envoyproxy_protoc_gen_validate_validate_validate
#define _H_intri_pb_github_com_envoyproxy_protoc_gen_validate_validate_validate

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../github.com/golang/protobuf/ptypes/duration/duration.pb.h"
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

// WellKnownRegex contain some well-known patterns.
enum validatepb_KnownRegex {
  validatepb_KnownRegex_UNKNOWN = 0,
  // HTTP header name as defined by RFC 7230.
  validatepb_KnownRegex_HTTP_HEADER_NAME = 1,
  // HTTP header value as defined by RFC 7230.
  validatepb_KnownRegex_HTTP_HEADER_VALUE = 2,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

// FieldRules encapsulates the rules for each type of field. Depending on the
// field, the correct set should be used to ensure proper validations.
enum validatepb_FieldRules_Type_Union_Options {
  validatepb_FieldRules_Type_Union_Options_Float,
  validatepb_FieldRules_Type_Union_Options_Double,
  validatepb_FieldRules_Type_Union_Options_Int32,
  validatepb_FieldRules_Type_Union_Options_Int64,
  validatepb_FieldRules_Type_Union_Options_Uint32,
  validatepb_FieldRules_Type_Union_Options_Uint64,
  validatepb_FieldRules_Type_Union_Options_Sint32,
  validatepb_FieldRules_Type_Union_Options_Sint64,
  validatepb_FieldRules_Type_Union_Options_Fixed32,
  validatepb_FieldRules_Type_Union_Options_Fixed64,
  validatepb_FieldRules_Type_Union_Options_Sfixed32,
  validatepb_FieldRules_Type_Union_Options_Sfixed64,
  validatepb_FieldRules_Type_Union_Options_Bool,
  validatepb_FieldRules_Type_Union_Options_String_,
  validatepb_FieldRules_Type_Union_Options_Bytes,
  validatepb_FieldRules_Type_Union_Options_Enum,
  validatepb_FieldRules_Type_Union_Options_Repeated,
  validatepb_FieldRules_Type_Union_Options_Map,
  validatepb_FieldRules_Type_Union_Options_Any,
  validatepb_FieldRules_Type_Union_Options_Duration,
  validatepb_FieldRules_Type_Union_Options_Timestamp,
};
struct validatepb_FieldRules {
  struct validatepb_MessageRules *Message;
  enum validatepb_FieldRules_Type_Union_Options Type_Union_Option;
  union {
    // Scalar Field Types
    struct validatepb_FloatRules *Type_Float;
    struct validatepb_DoubleRules *Type_Double;
    struct validatepb_Int32Rules *Type_Int32;
    struct validatepb_Int64Rules *Type_Int64;
    struct validatepb_UInt32Rules *Type_Uint32;
    struct validatepb_UInt64Rules *Type_Uint64;
    struct validatepb_SInt32Rules *Type_Sint32;
    struct validatepb_SInt64Rules *Type_Sint64;
    struct validatepb_Fixed32Rules *Type_Fixed32;
    struct validatepb_Fixed64Rules *Type_Fixed64;
    struct validatepb_SFixed32Rules *Type_Sfixed32;
    struct validatepb_SFixed64Rules *Type_Sfixed64;
    struct validatepb_BoolRules *Type_Bool;
    struct validatepb_StringRules *Type_String;
    struct validatepb_BytesRules *Type_Bytes;
    // Complex Field Types
    struct validatepb_EnumRules *Type_Enum;
    struct validatepb_RepeatedRules *Type_Repeated;
    struct validatepb_MapRules *Type_Map;
    // Well-Known Field Types
    struct validatepb_AnyRules *Type_Any;
    struct validatepb_DurationRules *Type_Duration;
    struct validatepb_TimestampRules *Type_Timestamp;
  } Type;
};

// FloatRules describes the constraints applied to `float` values
struct validatepb_FloatRules {
  // Const specifies that this field must be exactly the specified value
  float Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  float Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  float Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  float Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  float Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  float *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  float *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// DoubleRules describes the constraints applied to `double` values
struct validatepb_DoubleRules {
  // Const specifies that this field must be exactly the specified value
  double Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  double Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  double Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  double Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  double Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  double *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  double *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// Int32Rules describes the constraints applied to `int32` values
struct validatepb_Int32Rules {
  // Const specifies that this field must be exactly the specified value
  int32_t Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  int32_t Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  int32_t Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  int32_t Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  int32_t Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  int32_t *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  int32_t *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// Int64Rules describes the constraints applied to `int64` values
struct validatepb_Int64Rules {
  // Const specifies that this field must be exactly the specified value
  int64_t Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  int64_t Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  int64_t Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  int64_t Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  int64_t Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  int64_t *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  int64_t *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// UInt32Rules describes the constraints applied to `uint32` values
struct validatepb_UInt32Rules {
  // Const specifies that this field must be exactly the specified value
  uint32_t Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  uint32_t Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  uint32_t Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  uint32_t Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  uint32_t Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  uint32_t *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  uint32_t *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// UInt64Rules describes the constraints applied to `uint64` values
struct validatepb_UInt64Rules {
  // Const specifies that this field must be exactly the specified value
  uint64_t Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  uint64_t Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  uint64_t Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  uint64_t Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  uint64_t Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  uint64_t *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  uint64_t *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// SInt32Rules describes the constraints applied to `sint32` values
struct validatepb_SInt32Rules {
  // Const specifies that this field must be exactly the specified value
  int32_t Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  int32_t Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  int32_t Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  int32_t Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  int32_t Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  int32_t *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  int32_t *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// SInt64Rules describes the constraints applied to `sint64` values
struct validatepb_SInt64Rules {
  // Const specifies that this field must be exactly the specified value
  int64_t Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  int64_t Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  int64_t Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  int64_t Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  int64_t Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  int64_t *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  int64_t *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// Fixed32Rules describes the constraints applied to `fixed32` values
struct validatepb_Fixed32Rules {
  // Const specifies that this field must be exactly the specified value
  uint32_t Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  uint32_t Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  uint32_t Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  uint32_t Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  uint32_t Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  uint32_t *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  uint32_t *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// Fixed64Rules describes the constraints applied to `fixed64` values
struct validatepb_Fixed64Rules {
  // Const specifies that this field must be exactly the specified value
  uint64_t Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  uint64_t Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  uint64_t Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  uint64_t Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  uint64_t Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  uint64_t *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  uint64_t *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// SFixed32Rules describes the constraints applied to `sfixed32` values
struct validatepb_SFixed32Rules {
  // Const specifies that this field must be exactly the specified value
  int32_t Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  int32_t Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  int32_t Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  int32_t Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  int32_t Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  int32_t *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  int32_t *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// SFixed64Rules describes the constraints applied to `sfixed64` values
struct validatepb_SFixed64Rules {
  // Const specifies that this field must be exactly the specified value
  int64_t Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  int64_t Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  int64_t Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  int64_t Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  int64_t Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  int64_t *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  int64_t *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// BoolRules describes the constraints applied to `bool` values
struct validatepb_BoolRules {
  // Const specifies that this field must be exactly the specified value
  bool Const;
};

// StringRules describe the constraints applied to `string` values
enum validatepb_StringRules_WellKnown_Union_Options {
  validatepb_StringRules_WellKnown_Union_Options_Email,
  validatepb_StringRules_WellKnown_Union_Options_Hostname,
  validatepb_StringRules_WellKnown_Union_Options_Ip,
  validatepb_StringRules_WellKnown_Union_Options_Ipv4,
  validatepb_StringRules_WellKnown_Union_Options_Ipv6,
  validatepb_StringRules_WellKnown_Union_Options_Uri,
  validatepb_StringRules_WellKnown_Union_Options_UriRef,
  validatepb_StringRules_WellKnown_Union_Options_Address,
  validatepb_StringRules_WellKnown_Union_Options_Uuid,
  validatepb_StringRules_WellKnown_Union_Options_WellKnownRegex,
};
struct validatepb_StringRules {
  // Const specifies that this field must be exactly the specified value
  char *Const;
  // Len specifies that this field must be the specified number of
// characters (Unicode code points). Note that the number of
// characters may differ from the number of bytes in the string.
  uint64_t Len;
  // MinLen specifies that this field must be the specified number of
// characters (Unicode code points) at a minimum. Note that the number of
// characters may differ from the number of bytes in the string.
  uint64_t MinLen;
  // MaxLen specifies that this field must be the specified number of
// characters (Unicode code points) at a maximum. Note that the number of
// characters may differ from the number of bytes in the string.
  uint64_t MaxLen;
  // LenBytes specifies that this field must be the specified number of bytes
// at a minimum
  uint64_t LenBytes;
  // MinBytes specifies that this field must be the specified number of bytes
// at a minimum
  uint64_t MinBytes;
  // MaxBytes specifies that this field must be the specified number of bytes
// at a maximum
  uint64_t MaxBytes;
  // Pattern specifes that this field must match against the specified
// regular expression (RE2 syntax). The included expression should elide
// any delimiters.
  char *Pattern;
  // Prefix specifies that this field must have the specified substring at
// the beginning of the string.
  char *Prefix;
  // Suffix specifies that this field must have the specified substring at
// the end of the string.
  char *Suffix;
  // Contains specifies that this field must have the specified substring
// anywhere in the string.
  char *Contains;
  // NotContains specifies that this field cannot have the specified substring
// anywhere in the string.
  char *NotContains;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  char **In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  char **NotIn;
  // This applies to regexes HTTP_HEADER_NAME and HTTP_HEADER_VALUE to enable
// strict header validation.
// By default, this is true, and HTTP header validations are RFC-compliant.
// Setting to false will enable a looser validations that only disallows
// \r\n\0 characters, which can be used to bypass header matching rules.
  bool Strict;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
  enum validatepb_StringRules_WellKnown_Union_Options WellKnown_Union_Option;
  union {
    // Email specifies that the field must be a valid email address as
// defined by RFC 5322
    bool WellKnown_Email;
    // Hostname specifies that the field must be a valid hostname as
// defined by RFC 1034. This constraint does not support
// internationalized domain names (IDNs).
    bool WellKnown_Hostname;
    // Ip specifies that the field must be a valid IP (v4 or v6) address.
// Valid IPv6 addresses should not include surrounding square brackets.
    bool WellKnown_Ip;
    // Ipv4 specifies that the field must be a valid IPv4 address.
    bool WellKnown_Ipv4;
    // Ipv6 specifies that the field must be a valid IPv6 address. Valid
// IPv6 addresses should not include surrounding square brackets.
    bool WellKnown_Ipv6;
    // Uri specifies that the field must be a valid, absolute URI as defined
// by RFC 3986
    bool WellKnown_Uri;
    // UriRef specifies that the field must be a valid URI as defined by RFC
// 3986 and may be relative or absolute.
    bool WellKnown_UriRef;
    // Address specifies that the field must be either a valid hostname as
// defined by RFC 1034 (which does not support internationalized domain
// names or IDNs), or it can be a valid IP (v4 or v6).
    bool WellKnown_Address;
    // Uuid specifies that the field must be a valid UUID as defined by
// RFC 4122
    bool WellKnown_Uuid;
    // WellKnownRegex specifies a common well known pattern defined as a regex.
    enum validatepb_KnownRegex WellKnown_WellKnownRegex;
  } WellKnown;
};

// BytesRules describe the constraints applied to `bytes` values
enum validatepb_BytesRules_WellKnown_Union_Options {
  validatepb_BytesRules_WellKnown_Union_Options_Ip,
  validatepb_BytesRules_WellKnown_Union_Options_Ipv4,
  validatepb_BytesRules_WellKnown_Union_Options_Ipv6,
};
struct validatepb_BytesRules {
  // Const specifies that this field must be exactly the specified value
  unsigned char *Const;
  // Len specifies that this field must be the specified number of bytes
  uint64_t Len;
  // MinLen specifies that this field must be the specified number of bytes
// at a minimum
  uint64_t MinLen;
  // MaxLen specifies that this field must be the specified number of bytes
// at a maximum
  uint64_t MaxLen;
  // Pattern specifes that this field must match against the specified
// regular expression (RE2 syntax). The included expression should elide
// any delimiters.
  char *Pattern;
  // Prefix specifies that this field must have the specified bytes at the
// beginning of the string.
  unsigned char *Prefix;
  // Suffix specifies that this field must have the specified bytes at the
// end of the string.
  unsigned char *Suffix;
  // Contains specifies that this field must have the specified bytes
// anywhere in the string.
  unsigned char *Contains;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  unsigned char **In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  unsigned char **NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
  enum validatepb_BytesRules_WellKnown_Union_Options WellKnown_Union_Option;
  union {
    // Ip specifies that the field must be a valid IP (v4 or v6) address in
// byte format
    bool WellKnown_Ip;
    // Ipv4 specifies that the field must be a valid IPv4 address in byte
// format
    bool WellKnown_Ipv4;
    // Ipv6 specifies that the field must be a valid IPv6 address in byte
// format
    bool WellKnown_Ipv6;
  } WellKnown;
};

// EnumRules describe the constraints applied to enum values
struct validatepb_EnumRules {
  // Const specifies that this field must be exactly the specified value
  int32_t Const;
  // DefinedOnly specifies that this field must be only one of the defined
// values for this enum, failing on any undefined value.
  bool DefinedOnly;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  int32_t *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  int32_t *NotIn;
};

// MessageRules describe the constraints applied to embedded message values.
// For message-type fields, validation is performed recursively.
struct validatepb_MessageRules {
  // Skip specifies that the validation rules of this field should not be
// evaluated
  bool Skip;
  // Required specifies that this field must be set
  bool Required;
};

// RepeatedRules describe the constraints applied to `repeated` values
struct validatepb_RepeatedRules {
  // MinItems specifies that this field must have the specified number of
// items at a minimum
  uint64_t MinItems;
  // MaxItems specifies that this field must have the specified number of
// items at a maximum
  uint64_t MaxItems;
  // Unique specifies that all elements in this field must be unique. This
// contraint is only applicable to scalar and enum types (messages are not
// supported).
  bool Unique;
  // Items specifies the contraints to be applied to each item in the field.
// Repeated message fields will still execute validation against each item
// unless skip is specified here.
  struct validatepb_FieldRules *Items;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// MapRules describe the constraints applied to `map` values
struct validatepb_MapRules {
  // MinPairs specifies that this field must have the specified number of
// KVs at a minimum
  uint64_t MinPairs;
  // MaxPairs specifies that this field must have the specified number of
// KVs at a maximum
  uint64_t MaxPairs;
  // NoSparse specifies values in this field cannot be unset. This only
// applies to map's with message value types.
  bool NoSparse;
  // Keys specifies the constraints to be applied to each key in the field.
  struct validatepb_FieldRules *Keys;
  // Values specifies the constraints to be applied to the value of each key
// in the field. Message values will still have their validations evaluated
// unless skip is specified here.
  struct validatepb_FieldRules *Values;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// AnyRules describe constraints applied exclusively to the
// `google.protobuf.Any` well-known type
struct validatepb_AnyRules {
  // Required specifies that this field must be set
  bool Required;
  // In specifies that this field's `type_url` must be equal to one of the
// specified values.
  unsigned int In_Len; // auto-gen: for list
  char **In;
  // NotIn specifies that this field's `type_url` must not be equal to any of
// the specified values.
  unsigned int NotIn_Len; // auto-gen: for list
  char **NotIn;
};

// DurationRules describe the constraints applied exclusively to the
// `google.protobuf.Duration` well-known type
struct validatepb_DurationRules {
  // Required specifies that this field must be set
  bool Required;
  // Const specifies that this field must be exactly the specified value
  struct durationpb_Duration *Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  struct durationpb_Duration *Lt;
  // Lt specifies that this field must be less than the specified value,
// inclusive
  struct durationpb_Duration *Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive
  struct durationpb_Duration *Gt;
  // Gte specifies that this field must be greater than the specified value,
// inclusive
  struct durationpb_Duration *Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  struct durationpb_Duration **In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  struct durationpb_Duration **NotIn;
};

// TimestampRules describe the constraints applied exclusively to the
// `google.protobuf.Timestamp` well-known type
struct validatepb_TimestampRules {
  // Required specifies that this field must be set
  bool Required;
  // Const specifies that this field must be exactly the specified value
  struct timestamppb_Timestamp *Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  struct timestamppb_Timestamp *Lt;
  // Lte specifies that this field must be less than the specified value,
// inclusive
  struct timestamppb_Timestamp *Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive
  struct timestamppb_Timestamp *Gt;
  // Gte specifies that this field must be greater than the specified value,
// inclusive
  struct timestamppb_Timestamp *Gte;
  // LtNow specifies that this must be less than the current time. LtNow
// can only be used with the Within rule.
  bool LtNow;
  // GtNow specifies that this must be greater than the current time. GtNow
// can only be used with the Within rule.
  bool GtNow;
  // Within specifies that this field must be within this duration of the
// current time. This constraint can be used alone or with the LtNow and
// GtNow rules.
  struct durationpb_Duration *Within;
};
#endif // _H_intri_pb_github_com_envoyproxy_protoc_gen_validate_validate_validate
