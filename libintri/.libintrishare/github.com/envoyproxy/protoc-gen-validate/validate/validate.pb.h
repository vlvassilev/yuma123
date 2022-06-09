
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
struct validatepb_FieldRules {
  struct validatepb_MessageRules *Message;
  union {
    // Scalar Field Types
    struct validatepb_FloatRules *FieldRules_Type_Float;
    struct validatepb_DoubleRules *FieldRules_Type_Double;
    struct validatepb_Int32Rules *FieldRules_Type_Int32;
    struct validatepb_Int64Rules *FieldRules_Type_Int64;
    struct validatepb_UInt32Rules *FieldRules_Type_Uint32;
    struct validatepb_UInt64Rules *FieldRules_Type_Uint64;
    struct validatepb_SInt32Rules *FieldRules_Type_Sint32;
    struct validatepb_SInt64Rules *FieldRules_Type_Sint64;
    struct validatepb_Fixed32Rules *FieldRules_Type_Fixed32;
    struct validatepb_Fixed64Rules *FieldRules_Type_Fixed64;
    struct validatepb_SFixed32Rules *FieldRules_Type_Sfixed32;
    struct validatepb_SFixed64Rules *FieldRules_Type_Sfixed64;
    struct validatepb_BoolRules *FieldRules_Type_Bool;
    struct validatepb_StringRules *FieldRules_Type_String;
    struct validatepb_BytesRules *FieldRules_Type_Bytes;
    // Complex Field Types
    struct validatepb_EnumRules *FieldRules_Type_Enum;
    struct validatepb_RepeatedRules *FieldRules_Type_Repeated;
    struct validatepb_MapRules *FieldRules_Type_Map;
    // Well-Known Field Types
    struct validatepb_AnyRules *FieldRules_Type_Any;
    struct validatepb_DurationRules *FieldRules_Type_Duration;
    struct validatepb_TimestampRules *FieldRules_Type_Timestamp;
  };
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
  long int Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  long int Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  long int Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  long int Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  long int Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  long int *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  long int *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// Int64Rules describes the constraints applied to `int64` values
struct validatepb_Int64Rules {
  // Const specifies that this field must be exactly the specified value
  long long int Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  long long int Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  long long int Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  long long int Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  long long int Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  long long int *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  long long int *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// UInt32Rules describes the constraints applied to `uint32` values
struct validatepb_UInt32Rules {
  // Const specifies that this field must be exactly the specified value
  unsigned long int Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  unsigned long int Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  unsigned long int Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  unsigned long int Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  unsigned long int Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  unsigned long int *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  unsigned long int *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// UInt64Rules describes the constraints applied to `uint64` values
struct validatepb_UInt64Rules {
  // Const specifies that this field must be exactly the specified value
  unsigned long long int Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  unsigned long long int Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  unsigned long long int Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  unsigned long long int Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  unsigned long long int Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  unsigned long long int *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  unsigned long long int *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// SInt32Rules describes the constraints applied to `sint32` values
struct validatepb_SInt32Rules {
  // Const specifies that this field must be exactly the specified value
  long int Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  long int Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  long int Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  long int Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  long int Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  long int *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  long int *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// SInt64Rules describes the constraints applied to `sint64` values
struct validatepb_SInt64Rules {
  // Const specifies that this field must be exactly the specified value
  long long int Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  long long int Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  long long int Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  long long int Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  long long int Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  long long int *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  long long int *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// Fixed32Rules describes the constraints applied to `fixed32` values
struct validatepb_Fixed32Rules {
  // Const specifies that this field must be exactly the specified value
  unsigned long int Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  unsigned long int Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  unsigned long int Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  unsigned long int Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  unsigned long int Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  unsigned long int *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  unsigned long int *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// Fixed64Rules describes the constraints applied to `fixed64` values
struct validatepb_Fixed64Rules {
  // Const specifies that this field must be exactly the specified value
  unsigned long long int Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  unsigned long long int Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  unsigned long long int Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  unsigned long long int Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  unsigned long long int Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  unsigned long long int *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  unsigned long long int *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// SFixed32Rules describes the constraints applied to `sfixed32` values
struct validatepb_SFixed32Rules {
  // Const specifies that this field must be exactly the specified value
  long int Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  long int Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  long int Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  long int Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  long int Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  long int *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  long int *NotIn;
  // IgnoreEmpty specifies that the validation rules of this field should be
// evaluated only if the field is not empty
  bool IgnoreEmpty;
};

// SFixed64Rules describes the constraints applied to `sfixed64` values
struct validatepb_SFixed64Rules {
  // Const specifies that this field must be exactly the specified value
  long long int Const;
  // Lt specifies that this field must be less than the specified value,
// exclusive
  long long int Lt;
  // Lte specifies that this field must be less than or equal to the
// specified value, inclusive
  long long int Lte;
  // Gt specifies that this field must be greater than the specified value,
// exclusive. If the value of Gt is larger than a specified Lt or Lte, the
// range is reversed.
  long long int Gt;
  // Gte specifies that this field must be greater than or equal to the
// specified value, inclusive. If the value of Gte is larger than a
// specified Lt or Lte, the range is reversed.
  long long int Gte;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  long long int *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  long long int *NotIn;
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
struct validatepb_StringRules {
  // Const specifies that this field must be exactly the specified value
  char *Const;
  // Len specifies that this field must be the specified number of
// characters (Unicode code points). Note that the number of
// characters may differ from the number of bytes in the string.
  unsigned long long int Len;
  // MinLen specifies that this field must be the specified number of
// characters (Unicode code points) at a minimum. Note that the number of
// characters may differ from the number of bytes in the string.
  unsigned long long int MinLen;
  // MaxLen specifies that this field must be the specified number of
// characters (Unicode code points) at a maximum. Note that the number of
// characters may differ from the number of bytes in the string.
  unsigned long long int MaxLen;
  // LenBytes specifies that this field must be the specified number of bytes
// at a minimum
  unsigned long long int LenBytes;
  // MinBytes specifies that this field must be the specified number of bytes
// at a minimum
  unsigned long long int MinBytes;
  // MaxBytes specifies that this field must be the specified number of bytes
// at a maximum
  unsigned long long int MaxBytes;
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
  union {
    // Email specifies that the field must be a valid email address as
// defined by RFC 5322
    bool StringRules_WellKnown_Email;
    // Hostname specifies that the field must be a valid hostname as
// defined by RFC 1034. This constraint does not support
// internationalized domain names (IDNs).
    bool StringRules_WellKnown_Hostname;
    // Ip specifies that the field must be a valid IP (v4 or v6) address.
// Valid IPv6 addresses should not include surrounding square brackets.
    bool StringRules_WellKnown_Ip;
    // Ipv4 specifies that the field must be a valid IPv4 address.
    bool StringRules_WellKnown_Ipv4;
    // Ipv6 specifies that the field must be a valid IPv6 address. Valid
// IPv6 addresses should not include surrounding square brackets.
    bool StringRules_WellKnown_Ipv6;
    // Uri specifies that the field must be a valid, absolute URI as defined
// by RFC 3986
    bool StringRules_WellKnown_Uri;
    // UriRef specifies that the field must be a valid URI as defined by RFC
// 3986 and may be relative or absolute.
    bool StringRules_WellKnown_UriRef;
    // Address specifies that the field must be either a valid hostname as
// defined by RFC 1034 (which does not support internationalized domain
// names or IDNs), or it can be a valid IP (v4 or v6).
    bool StringRules_WellKnown_Address;
    // Uuid specifies that the field must be a valid UUID as defined by
// RFC 4122
    bool StringRules_WellKnown_Uuid;
    // WellKnownRegex specifies a common well known pattern defined as a regex.
    enum validatepb_KnownRegex StringRules_WellKnown_WellKnownRegex;
  };
};

// BytesRules describe the constraints applied to `bytes` values
struct validatepb_BytesRules {
  // Const specifies that this field must be exactly the specified value
  unsigned char *Const;
  // Len specifies that this field must be the specified number of bytes
  unsigned long long int Len;
  // MinLen specifies that this field must be the specified number of bytes
// at a minimum
  unsigned long long int MinLen;
  // MaxLen specifies that this field must be the specified number of bytes
// at a maximum
  unsigned long long int MaxLen;
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
  union {
    // Ip specifies that the field must be a valid IP (v4 or v6) address in
// byte format
    bool BytesRules_WellKnown_Ip;
    // Ipv4 specifies that the field must be a valid IPv4 address in byte
// format
    bool BytesRules_WellKnown_Ipv4;
    // Ipv6 specifies that the field must be a valid IPv6 address in byte
// format
    bool BytesRules_WellKnown_Ipv6;
  };
};

// EnumRules describe the constraints applied to enum values
struct validatepb_EnumRules {
  // Const specifies that this field must be exactly the specified value
  long int Const;
  // DefinedOnly specifies that this field must be only one of the defined
// values for this enum, failing on any undefined value.
  bool DefinedOnly;
  // In specifies that this field must be equal to one of the specified
// values
  unsigned int In_Len; // auto-gen: for list
  long int *In;
  // NotIn specifies that this field cannot be equal to one of the specified
// values
  unsigned int NotIn_Len; // auto-gen: for list
  long int *NotIn;
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
  unsigned long long int MinItems;
  // MaxItems specifies that this field must have the specified number of
// items at a maximum
  unsigned long long int MaxItems;
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
  unsigned long long int MinPairs;
  // MaxPairs specifies that this field must have the specified number of
// KVs at a maximum
  unsigned long long int MaxPairs;
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
