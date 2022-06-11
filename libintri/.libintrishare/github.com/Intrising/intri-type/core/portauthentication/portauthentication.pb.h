
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_portauthentication_portauthentication
#define _H_intri_pb_github_com_Intrising_intri_type_core_portauthentication_portauthentication

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/common/common.pb.h"
#include "../../../../../github.com/Intrising/intri-type/core/access/access.pb.h"
#include "../../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>
#include <stdint.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum portauthenticationpb_AuthorizationStateTypeOptions {
  // [Undefined]
  portauthenticationpb_AuthorizationStateTypeOptions_AUTHORIZATION_STATE_TYPE_UNDEFINED = 0,
  // [Disabled]
  portauthenticationpb_AuthorizationStateTypeOptions_AUTHORIZATION_STATE_TYPE_DISABLED = 1,
  // [Unauthorized]
  portauthenticationpb_AuthorizationStateTypeOptions_AUTHORIZATION_STATE_TYPE_UNAUTHORIZED = 2,
  // [Processing]
  portauthenticationpb_AuthorizationStateTypeOptions_AUTHORIZATION_STATE_TYPE_PROCESSING = 3,
  // [Authorized]
  portauthenticationpb_AuthorizationStateTypeOptions_AUTHORIZATION_STATE_TYPE_AUTHORIZED = 4,
  // [Rejected]
  portauthenticationpb_AuthorizationStateTypeOptions_AUTHORIZATION_STATE_TYPE_REJECTED = 5,
};

enum portauthenticationpb_AuthorizationModeTypeOptions {
  // [Always Auth]
  portauthenticationpb_AuthorizationModeTypeOptions_AUTHORIZATION_MODE_TYPE_ALWAYSAUTH = 0,
  // [Force Unauth]
  portauthenticationpb_AuthorizationModeTypeOptions_AUTHORIZATION_MODE_TYPE_FORCEUNAUTH = 1,
  // [Via MAC Table]
  portauthenticationpb_AuthorizationModeTypeOptions_AUTHORIZATION_MODE_TYPE_VIAMACTABLE = 2,
  // [MAC via RADIUS]
  portauthenticationpb_AuthorizationModeTypeOptions_AUTHORIZATION_MODE_TYPE_MACVIARADIUS = 3,
  // [802.1X via RADIUS]
  portauthenticationpb_AuthorizationModeTypeOptions_AUTHORIZATION_MODE_TYPE_8021XVIARADIUS = 4,
  // [MAC 802.1X via RADIUS]
  portauthenticationpb_AuthorizationModeTypeOptions_AUTHORIZATION_MODE_TYPE_MAC8021XVIARADIUS = 5,
  // [802.1X MAC via RADIUS]
  portauthenticationpb_AuthorizationModeTypeOptions_AUTHORIZATION_MODE_TYPE_8021XMACVIARADIUS = 6,
  // *internal
  portauthenticationpb_AuthorizationModeTypeOptions_AUTHORIZATION_MODE_TYPE_NONE = 9,
};

enum portauthenticationpb_UnAuthorizationModeTypeOptions {
  // [Blocked]
  portauthenticationpb_UnAuthorizationModeTypeOptions_UNAUTHORIZATION_MODE_TYPE_BLOCKED = 0,
  // [Use Unauthorized VLAN]
  portauthenticationpb_UnAuthorizationModeTypeOptions_UNAUTHORIZATION_MODE_TYPE_USE_UNAUTHORIZEDVLAN = 1,
  // [Incoming Blocked]
  portauthenticationpb_UnAuthorizationModeTypeOptions_UNAUTHORIZATION_MODE_TYPE_INCOMING_BLOCKED = 2,
};

enum portauthenticationpb_MACSpellingTypeOptions {
  // [Lowercase]
  portauthenticationpb_MACSpellingTypeOptions_MAC_SPELLING_TYPE_LOWERCASE = 0,
  // [Uppercase]
  portauthenticationpb_MACSpellingTypeOptions_MAC_SPELLING_TYPE_UPPERCASE = 1,
};

enum portauthenticationpb_MACPasswordSourceTypeOptions {
  // [Use MAC]
  portauthenticationpb_MACPasswordSourceTypeOptions_MAC_PASSWORD_SOURCE_TYPE_USEMAC = 0,
  // [Use Password]
  portauthenticationpb_MACPasswordSourceTypeOptions_MAC_PASSWORD_SOURCE_TYPE_USEPASSWORD = 1,
};

enum portauthenticationpb_EntryStateTypeOptions {
  // [Unused]
  portauthenticationpb_EntryStateTypeOptions_ENTRY_STATE_TYPE_UNUSED = 0,
  // [Inactive]
  portauthenticationpb_EntryStateTypeOptions_ENTRY_STATE_TYPE_INACTIVE = 1,
  // [Active]
  portauthenticationpb_EntryStateTypeOptions_ENTRY_STATE_TYPE_ACTIVE = 2,
};

enum portauthenticationpb_MACTimeoutTypeOptions {
  // [None]
  portauthenticationpb_MACTimeoutTypeOptions_MAC_TIMEOUT_TYPE_NONE = 0,
  // [Slow]
  portauthenticationpb_MACTimeoutTypeOptions_MAC_TIMEOUT_TYPE_SLOW = 1,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct portauthenticationpb_Config {
  struct portauthenticationpb_SystemConfig *SystemConfig;
  struct portauthenticationpb_PortConfig *PortConfig;
  struct portauthenticationpb_AuthorizedMACs *AuthorizedMACs;
};

struct portauthenticationpb_AuthorizedMACsEntry {
  // name + m_a_c_address is unique (add/update)
  char *Name;
  char *MACAddress;
  // only for physical ports
  unsigned int PermittedPortList_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **PermittedPortList;
  bool TreatAsVendorMAC;
};

struct portauthenticationpb_AuthorizedMACs {
  unsigned int List_Len; // auto-gen: for list
  struct portauthenticationpb_AuthorizedMACsEntry **List;
};

struct portauthenticationpb_SystemConfig {
  bool PortAccessControlEnabled;
  int32_t ReauthenticationPeriod;
  char *NASIdentifier;
  char *MACSeparatorChar;
  enum portauthenticationpb_MACSpellingTypeOptions MACSpelling;
  enum portauthenticationpb_MACPasswordSourceTypeOptions MACPasswordSource;
  char *MACPasswordString;
  char *PrimaryAuthServerName;
  char *PrimaryAcctServerName;
  char *FallbackAuthServerName;
  char *FallbackAcctServerName;
  int32_t ServerDownTimeout;
};

struct portauthenticationpb_PortConfig {
  // boundary is according to device.GetPortLists()
  unsigned int List_Len; // auto-gen: for list
  struct portauthenticationpb_PortConfigEntry **List;
};

struct portauthenticationpb_PortConfigEntry {
  // Index to update, only support Physical port
  struct devicepb_InterfaceIdentify *IdentifyNo;
  enum portauthenticationpb_AuthorizationModeTypeOptions AuthorizedMode;
  enum portauthenticationpb_UnAuthorizationModeTypeOptions UnAuthorizedMode;
  int32_t AuthFailRetryInterval;
  int32_t LimitedNumberOfMACs;
  enum portauthenticationpb_MACTimeoutTypeOptions MACTimeout;
};

struct portauthenticationpb_PortStatus {
  unsigned int List_Len; // auto-gen: for list
  struct portauthenticationpb_PortStatusEntry **List;
};

struct portauthenticationpb_PortStatusEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  enum portauthenticationpb_AuthorizationStateTypeOptions AuthorizationState;
  enum portauthenticationpb_AuthorizationModeTypeOptions AuthorizationMode;
  char *LastStateChange;
  int32_t NumberOfMACsToLearn;
  int32_t NumberOfLearnedMACs;
};

struct portauthenticationpb_PortAuthorizationStatus {
  unsigned int List_Len; // auto-gen: for list
  struct portauthenticationpb_PortAuthorizationStatusEntry **List;
};

struct portauthenticationpb_PortAuthorizationStatusEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  enum portauthenticationpb_AuthorizationStateTypeOptions AuthorizationState;
  char *UserMAC;
  char *UserName;
  // Not Support : 802.1X with VLAN assignment
// string vlan_alias = 5;
// int32 vlan_i_d = 6 [json_name = "vid"];
  int32_t IdleTimeout;
  int32_t SessionTimeout;
  // Not Support : 802.1X dynamic access control list(ACL) based on RADIUS attributes
// string filter_i_d = 9;
  char *LastStateChange;
};

struct portauthenticationpb_UserStatus {
  unsigned int List_Len; // auto-gen: for list
  struct portauthenticationpb_UserStatusEntry **List;
};

struct portauthenticationpb_UserStatusEntry {
  enum portauthenticationpb_EntryStateTypeOptions EntryState;
  enum portauthenticationpb_AuthorizationStateTypeOptions AuthorizationState;
  enum portauthenticationpb_AuthorizationModeTypeOptions AuthorizationMode;
  struct devicepb_InterfaceIdentify *IdentifyNo;
  char *UserMAC;
  char *UserName;
  // Not Support : 802.1X with VLAN assignment
// string vlan_alias = 7;
  int32_t VlanID;
  int32_t IdleTimeout;
  int32_t SessionTimeout;
  // Not Support : 802.1X dynamic access control list(ACL) based on RADIUS attributes
// string filter_i_d = 11;
  char *LoginTimeStamp;
};

struct portauthenticationpb_LearnMACNowEntry {
  // Index
  struct devicepb_InterfaceIdentify *IdentifyNo;
  int32_t Amount;
};

struct portauthenticationpb_UnauthorizeMACEntry {
  // Index
  struct devicepb_InterfaceIdentify *IdentifyNo;
  char *MACAddress;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_portauthentication_portauthentication
