
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_access_access
#define _H_intri_pb_github_com_Intrising_intri_type_core_access_access

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/common/common.pb.h"
#include "../../../../../github.com/Intrising/intri-type/event/event.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
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

enum accesspb_AuthenticationModeTypeOptions {
  // [Local] Verify against local database
  accesspb_AuthenticationModeTypeOptions_AUTHENTICATION_MODE_TYPE_LOCAL = 0,
  // [Local Then RADIUS] Verify against local database then try RADIUS server if user is not locally defined
  accesspb_AuthenticationModeTypeOptions_AUTHENTICATION_MODE_TYPE_LOCAL_THEN_RADIUS = 1,
  // [RADIUS] Verify against RADIUS server
  accesspb_AuthenticationModeTypeOptions_AUTHENTICATION_MODE_TYPE_RADIUS = 2,
  // [Local Then TACACS+] Verify against local database then try TACACS+ server if user is not locally defined
  accesspb_AuthenticationModeTypeOptions_AUTHENTICATION_MODE_TYPE_LOCAL_THEN_TACACS = 3,
  // [TACACS+] Verify against TACACS+ server
  accesspb_AuthenticationModeTypeOptions_AUTHENTICATION_MODE_TYPE_TACACS = 4,
  // [RADIUS Then Local] Verify against RADIUS server then try local database
  accesspb_AuthenticationModeTypeOptions_AUTHENTICATION_MODE_TYPE_RADIUS_THEN_LOCAL = 5,
  // [TACACS+ Then Local] Verify against TACACS+ server then try local database
  accesspb_AuthenticationModeTypeOptions_AUTHENTICATION_MODE_TYPE_TACACS_THEN_LOCAL = 6,
};

enum accesspb_AuthenticationServerTypeOptions {
  // [RADIUS]
  accesspb_AuthenticationServerTypeOptions_AUTHENTICATION_SERVER_TYPE_RADIUS = 0,
  // [TACACS+]
  accesspb_AuthenticationServerTypeOptions_AUTHENTICATION_SERVER_TYPE_TACACS = 1,
};

enum accesspb_RestrictionModeTypeOptions {
  // [Unused]
  accesspb_RestrictionModeTypeOptions_RESTRICTION_MODE_TYPE_UNUSED = 0,
  // [Permit]
  accesspb_RestrictionModeTypeOptions_RESTRICTION_MODE_TYPE_PERMIT = 1,
  // [Deny]
  accesspb_RestrictionModeTypeOptions_RESTRICTION_MODE_TYPE_DENY = 2,
};

enum accesspb_RightsTypeOptions {
  // [No Access]
  accesspb_RightsTypeOptions_RIGHTS_TYPE_NO_ACCESS = 0,
  // [Execute Only]
  accesspb_RightsTypeOptions_RIGHTS_TYPE_EXECUTE_ONLY = 1,
  // [Read Only]
  accesspb_RightsTypeOptions_RIGHTS_TYPE_READ_ONLY = 2,
  // [Read Write]
  accesspb_RightsTypeOptions_RIGHTS_TYPE_READ_WRITE = 3,
  // [Read Execute]
  accesspb_RightsTypeOptions_RIGHTS_TYPE_READ_EXECUTE = 4,
  // [Read Write Execute]
  accesspb_RightsTypeOptions_RIGHTS_TYPE_READ_WRITE_EXECUTE = 5,
};

enum accesspb_UserEntrySNMPV3SecurityLevelTypeOptions {
  // [None]
  accesspb_UserEntrySNMPV3SecurityLevelTypeOptions_USER_ENTRY_SNMP_V3_SECURITY_LEVEL_TYPE_NO_AUTH_NO_PRIV = 0,
  // [Only Auth]
  accesspb_UserEntrySNMPV3SecurityLevelTypeOptions_USER_ENTRY_SNMP_V3_SECURITY_LEVEL_TYPE_AUTH_NO_PRIV = 1,
  // [Auth & Privacy]
  accesspb_UserEntrySNMPV3SecurityLevelTypeOptions_USER_ENTRY_SNMP_V3_SECURITY_LEVEL_TYPE_AUTH_PRIV = 2,
};

enum accesspb_UserEntrySNMPV3AuthAlgorithmTypeOptions {
  // [None]
  accesspb_UserEntrySNMPV3AuthAlgorithmTypeOptions_USER_ENTRY_SNMP_V3_AUTH_ALGORITHM_TYPE_NO_AUTHENTICATION = 0,
  // [MD5]
  accesspb_UserEntrySNMPV3AuthAlgorithmTypeOptions_USER_ENTRY_SNMP_V3_AUTH_ALGORITHM_TYPE_MD5 = 1,
  // [SHA 1]
  accesspb_UserEntrySNMPV3AuthAlgorithmTypeOptions_USER_ENTRY_SNMP_V3_AUTH_ALGORITHM_TYPE_SHA_1 = 2,
  // [SHA 256]
  accesspb_UserEntrySNMPV3AuthAlgorithmTypeOptions_USER_ENTRY_SNMP_V3_AUTH_ALGORITHM_TYPE_SHA_256 = 3,
};

enum accesspb_UserEntrySNMPV3PrivacyAlgorithmTypeOptions {
  // [None]
  accesspb_UserEntrySNMPV3PrivacyAlgorithmTypeOptions_USER_ENTRY_SNMP_V3_PRIVACY_ALGORITHM_TYPE_NO_PRIVACY = 0,
  // [DES]
  accesspb_UserEntrySNMPV3PrivacyAlgorithmTypeOptions_USER_ENTRY_SNMP_V3_PRIVACY_ALGORITHM_TYPE_DES = 1,
  // [AES]
  accesspb_UserEntrySNMPV3PrivacyAlgorithmTypeOptions_USER_ENTRY_SNMP_V3_PRIVACY_ALGORITHM_TYPE_AES = 2,
  // [AES 192]
  accesspb_UserEntrySNMPV3PrivacyAlgorithmTypeOptions_USER_ENTRY_SNMP_V3_PRIVACY_ALGORITHM_TYPE_AES_192 = 3,
  // [AES 256]
  accesspb_UserEntrySNMPV3PrivacyAlgorithmTypeOptions_USER_ENTRY_SNMP_V3_PRIVACY_ALGORITHM_TYPE_AES_256 = 4,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct accesspb_AuthenticationMode {
  enum accesspb_AuthenticationModeTypeOptions Mode;
};

struct accesspb_AuthenticationConfig {
  enum accesspb_AuthenticationModeTypeOptions Mode;
  // when mode contains RADIUS, the field will be used
  struct accesspb_AuthenticationRADIUS *RADIUS;
  // when mode contains TACACS, the field will be used
  struct accesspb_AuthenticationTACACS *TACACS;
};

struct accesspb_AuthenticationTACACS {
  // the server name is selected by the response"AuthenticationServerEntry.Name" from rpc GetAuthenticatorServerConfig
  char *PrimaryAuthServerName;
  char *FallbackAuthServerName;
  // the user privilege is selected by the response"UserEntry.Name" from rpc GetUsers
  char *UserPrivilegeLevel0;
  char *UserPrivilegeLevel1;
  char *UserPrivilegeLevel15;
};

struct accesspb_AuthenticationRADIUS {
  // the server name is selected by the response"AuthenticationServerEntry.Name" from rpc GetAuthenticatorServerConfig
  char *PrimaryAuthServerName;
  char *FallbackAuthServerName;
  // the user privilege is selected by the response"UserEntry.Name" from rpc GetUsers
  char *UserPrivilegeLevel6;
  char *UserPrivilegeLevel7;
};

struct accesspb_AuthenticationServerEntry {
  // Index (for update / delete); unique. Only name is defined among AuthenticationServersConfig.
  char *Name;
  enum accesspb_AuthenticationServerTypeOptions ServerType;
  char *HostAddress;
  int32_t PortNumber;
  char *SharedSecret;
  int64_t InterimInterval;
};

struct accesspb_AuthenticationServersConfig {
  // boundary is according to device.GetBoundary().Access.Servers
  unsigned int List_Len; // auto-gen: for list
  struct accesspb_AuthenticationServerEntry **List;
};

struct accesspb_LoginRequest {
  char *Account;
  char *Password;
};

struct accesspb_RestrictionEntry {
  // Index (for update / delete); unique. Only name is defined among RestrictionsConfig.
  char *Name;
  enum accesspb_RestrictionModeTypeOptions Mode;
  char *IPAddress;
};

struct accesspb_RestrictionsConfig {
  // boundary is according to device.GetBoundary().Access.Restrictions
  unsigned int List_Len; // auto-gen: for list
  struct accesspb_RestrictionEntry **List;
};

struct accesspb_UserEntry {
  // Index (for update / delete); unique. Only name is defined among UsersConfig.
  char *Name;
  // boundary is according to device.GetBoundary().Access.UsersAssociatedGroups
  unsigned int AssociatedGroups_Len; // auto-gen: for list
  char **AssociatedGroups;
  enum accesspb_RightsTypeOptions GeneralAccessRights;
  bool TelnetAccessEnabled;
  bool SSHAccessEnabled;
  bool WebAccessEnabled;
  bool SNMPAccessEnabled;
  char *EncryptedAuthPassword;
  enum accesspb_UserEntrySNMPV3SecurityLevelTypeOptions SNMPV3SecurityLevel;
  enum accesspb_UserEntrySNMPV3AuthAlgorithmTypeOptions SNMPV3AuthAlgorithm;
  enum accesspb_UserEntrySNMPV3PrivacyAlgorithmTypeOptions SNMPV3PrivacyAlgorithm;
  char *EncryptedSNMPAuthPassword;
  char *EncryptedSNMPPrivacyPassword;
};

struct accesspb_PatternEntry {
  // Index (for update / delete); unique. Only path is defined among GroupEntry.Patterns.
  char *Path;
  enum accesspb_RightsTypeOptions AccessRight;
};

struct accesspb_UsersConfig {
  // boundary is according to device.GetBoundary().Access.Users
  unsigned int List_Len; // auto-gen: for list
  struct accesspb_UserEntry **List;
};

struct accesspb_GroupEntry {
  // Index (for update / delete); unique. Only name is defined among GroupsConfig.
  char *Name;
  // boundary is according to device.GetBoundary().Access.GroupsPattern
  unsigned int Patterns_Len; // auto-gen: for list
  struct accesspb_PatternEntry **Patterns;
};

struct accesspb_GroupsConfig {
  // boundary is according to device.GetBoundary().Access.Groups
  unsigned int List_Len; // auto-gen: for list
  struct accesspb_GroupEntry **List;
};

struct accesspb_Config {
  struct accesspb_AuthenticationConfig *Authentication;
  struct accesspb_UsersConfig *Users;
  struct accesspb_GroupsConfig *Groups;
  struct accesspb_RestrictionsConfig *Restrictions;
  struct accesspb_AuthenticationServersConfig *Servers;
};

// the Password is used to encrpyt
struct accesspb_Password {
  char *Password;
  // if the password is for user , the irreversable is true
// for others password, the irreversable is false
  bool Irreversable;
};

// the EncryptedPassword is used for encrypted passwords in UserEntry
struct accesspb_EncryptedPassword {
  char *EncryptedPassword;
};

// the UserPasswordEntry will be encrypted and update the password in config automatically
// the password is plantext
struct accesspb_UserPasswordEntry {
  char *Username;
  char *Password;
};

struct accesspb_UserPassword {
  // boundary is according to device.GetBoundary().Access.Users
  unsigned int List_Len; // auto-gen: for list
  struct accesspb_UserPasswordEntry **List;
};

// the below messages "NumberOfLogins,LoginStatusEntry,LoginStatus" are not used currently.
struct accesspb_NumberOfLogins {
  int32_t NumberOfLogins;
};

// each entry is unique.
struct accesspb_LoginStatusEntry {
  char *State;
  char *Username;
  char *AuthName;
  char *LoginID;
  char *LoginTimeStamp;
  char *LoginEpoch;
  int64_t ConnectTime;
  enum eventpb_LoginInterfaceNameTypeOptions Service;
};

struct accesspb_LoginStatus {
  unsigned int List_Len; // auto-gen: for list
  struct accesspb_LoginStatusEntry **List;
};

struct accesspb_Token {
  char *Token;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_access_access
