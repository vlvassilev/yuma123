
#ifndef _H_intri_pb_github_com_Intrising_intri_type_common_common
#define _H_intri_pb_github_com_Intrising_intri_type_common_common

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

enum commonpb_IPv6ScopeTypeOptions {
  // [Link]
  commonpb_IPv6ScopeTypeOptions_IPV6_SCOPE_TYPE_LINK = 0,
  // [Site]
  commonpb_IPv6ScopeTypeOptions_IPV6_SCOPE_TYPE_SITE = 1,
  // [Global]
  commonpb_IPv6ScopeTypeOptions_IPV6_SCOPE_TYPE_GLOBAL = 2,
  // [Other]
  commonpb_IPv6ScopeTypeOptions_IPV6_SCOPE_TYPE_OTHER = 3,
};

enum commonpb_VlanServiceErrorOptions {
  commonpb_VlanServiceErrorOptions_MAC_BASED_ADD_LIST_OUT_OF_RANGE = 0,
  commonpb_VlanServiceErrorOptions_MAC_BASED_ADD_GROUP_ID_OUT_OF_RANGE = 1,
  commonpb_VlanServiceErrorOptions_MAC_BASED_ADD_GROUP_ID_EXIST = 2,
  commonpb_VlanServiceErrorOptions_MAC_BASED_ADD_MAC_ADDRESS_INVALID = 3,
  commonpb_VlanServiceErrorOptions_MAC_BASED_ADD_MAC_ADDRESS_MASK_INVALID = 4,
};

enum commonpb_ServicesEnumTypeOptions {
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_ALL = 0,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_EVENT = 1,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_PACKET = 2,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CONFIG = 3,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_LOG = 4,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_DEVICE = 10,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_DEVICE_FUNCTION_CONTROL = 11,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS = 20,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_ACL = 21,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_DEVICE = 22,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_BOARD = 23,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_FDB = 24,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_MIRRORING = 25,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_MISC = 26,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_MULTICAST = 27,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_PORT = 28,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_QOS = 29,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_RATELIMITING = 30,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_STP = 31,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_TRUNKING = 32,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_VLAN = 33,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_PACKET_CONTROL = 34,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_PTP = 35,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_CNC = 36,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CPSS_POLICER = 37,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_HARDWARE = 40,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CLI = 41,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_SNMP = 42,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_WEB = 43,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_SSH = 44,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_TELNET = 45,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CONSOLE = 46,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_UNKNOWN = 47,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_GATEWAY = 48,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_RESTAPI = 49,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_VLAN = 50,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_VLAN_GVRP = 51,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_ACL = 60,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_ACL_STORMCONTROL = 61,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_ACCESS = 70,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_ACCESS_USER_INTERFACE = 71,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_AU = 80,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_AU_SECURITY = 81,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_AUTHENTICATION_PORT = 82,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_SYSTEM = 90,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_SYSTEM_DIAGNOSTIC = 91,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_SYSTEM_MAINTENANCE = 92,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_SYSTEM_TIME_RANGE = 93,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_SYSTEM_FILES = 94,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_LACP = 100,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_PORT = 110,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_PORT_RMON = 111,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_PORT_SFP = 112,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_PORT_ISOLATION = 113,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_PORT_MIRRORING = 114,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_MULTICAST = 120,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_DHCP = 130,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_QOS = 140,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_NETWORK = 150,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_TIME = 160,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_LOOP = 170,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_LLDP = 180,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_POE = 181,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_STP = 182,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_CDP = 183,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_HARDWARE_POE = 184,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_MONITOR = 185,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_UDLD = 186,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_CORE_PTP = 190,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_TIME_CONTROL = 200,
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_QT_CLI = 210,
  // add the same prefix with snmp
  commonpb_ServicesEnumTypeOptions_SERVICES_ENUM_TYPE_SNMP_TRAP = 221,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct commonpb_Path {
  char *Path;
};

struct commonpb_ExportPathRequest {
  char *Path;
  bool FTPSEnabled;
};

struct commonpb_ImportPathRequest {
  // The full path with the file name on the FTP server
// ex: ftp://192.168.1.1/firmware/dragonV10015.upg
// ex: ftps://192.168.1.1/config/setting.yml
// [NOTE]:
// 1. the path cannot contain a certain character
// ex: ";"
// 2. the regex of the path doesn't check the extension, core will check it.  https://github.com/Intrising/intri-type/issues/282
// 3. Example of the ftps/ftp/sftp
// FTP:
//     path: ftp://192.168.1.1/firmware/dragonV10015.upg
//     ftpsEnabled: false
// SFTP:
//     path: sftp://192.168.1.1/config/setting.yml
//     ftpsEnabled: false
// FTPS:
//     path: ftp://192.168.1.1/firmware/dragonV10015.upg
//     ftpsEnabled: true
  char *Path;
  bool FTPSEnabled;
  bool RebootAfterAction;
};

struct commonpb_Enabled {
  bool IsEnabled;
};

struct commonpb_MACAddress {
  char *MACAddr;
};

struct commonpb_IPAddress {
  char *IPAddr;
};

struct commonpb_IPv4List {
  unsigned int List_Len; // auto-gen: for list
  char **List;
};

struct commonpb_IPv6Entry {
  enum commonpb_IPv6ScopeTypeOptions Type;
  char *IPAddr;
};

struct commonpb_IPv6List {
  unsigned int List_Len; // auto-gen: for list
  struct commonpb_IPv6Entry **List;
};

struct commonpb_Name {
  char *Name;
};

struct commonpb_NameList {
  unsigned int NameList_Len; // auto-gen: for list
  char **NameList;
};

struct commonpb_Reply {
  char *Reply;
};

struct commonpb_Confirm {
  char *Confirm;
};

struct commonpb_DateTime {
  char *Ts;
};

struct commonpb_AllStatus {
  char *Status;
};

struct commonpb_State {
  bool State;
};

struct commonpb_Index {
  long int Index;
};

// ian: will be deprecated
struct commonpb_GRPCErrorDetail {
  char *FieldName;
  char *FieldValue;
  char *Description;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_common_common
