
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_portsecurity_portsecurity
#define _H_intri_pb_github_com_Intrising_intri_type_core_portsecurity_portsecurity

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum portsecuritypb_PortSecurityModeTypeOptions {
  // [Static]
  portsecuritypb_PortSecurityModeTypeOptions_PORT_SECURITY_MODE_TYPE_STATIC = 0,
  // [Sticky]
  portsecuritypb_PortSecurityModeTypeOptions_PORT_SECURITY_MODE_TYPE_STICKY = 1,
  // [Dynamic]
  portsecuritypb_PortSecurityModeTypeOptions_PORT_SECURITY_MODE_TYPE_DYNAMIC = 2,
};

enum portsecuritypb_PortSecurityAgeModeTypeOptions {
  // [Absolute] Absolute aging
  portsecuritypb_PortSecurityAgeModeTypeOptions_PORT_SECURITY_AGE_MODE_TYPE_ABSOULTE = 0,
  // [Inactivity] Aging based on inactivity time period
  portsecuritypb_PortSecurityAgeModeTypeOptions_PORT_SECURITY_AGE_MODE_TYPE_INACTIVITY = 1,
};

enum portsecuritypb_PortSecurityViolationTypeOptions {
  // [Shutdown] Security violation shutdown mode
  portsecuritypb_PortSecurityViolationTypeOptions_PORT_SECURITY_VIOLATION_TYPE_SHUTDOWN = 0,
  // [Discard] Security violation discard mode
  portsecuritypb_PortSecurityViolationTypeOptions_PORT_SECURITY_VIOLATION_TYPE_DISCARD = 1,
  // [Restrict] Security violation restrict mode
  portsecuritypb_PortSecurityViolationTypeOptions_PORT_SECURITY_VIOLATION_TYPE_RESTRICT = 2,
};

enum portsecuritypb_PortSecurityStatusTypeOptions {
  // [Secure-Down]
  portsecuritypb_PortSecurityStatusTypeOptions_PORT_SECURITY_STATUS_TYPE_SECURE_DOWN = 0,
  // [Secure-Up]
  portsecuritypb_PortSecurityStatusTypeOptions_PORT_SECURITY_STATUS_TYPE_SECURE_UP = 1,
  // [Secure-Shutdown]
  portsecuritypb_PortSecurityStatusTypeOptions_PORT_SECURITY_STATUS_TYPE_SECURE_SHUTDOWN = 2,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

// Port Security
struct portsecuritypb_Config {
  // boundary is according to device.GetBoundary().FDB.PortSecurityLearningLimit
  unsigned int List_Len; // auto-gen: for list
  struct portsecuritypb_PortSecurityConfigEntry **List;
};

struct portsecuritypb_PortSecurityConfigEntry {
  // Index (for update); unique. support INTERFACE_TYPE_PORT and INTERFACE_TYPE_TRUNK
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool Enabled;
  enum portsecuritypb_PortSecurityModeTypeOptions Mode;
  long int MaxMACCount;
  // if mode is PORT_SECURITY_MODE_TYPE_STATIC, the field can be configured, and the boundary depeneds on max_m_a_c_count
// if mode is PORT_SECURITY_MODE_TYPE_STICKY or PORT_SECURITY_MODE_TYPE_DYNAMIC, the field can not be configured
  unsigned int SecureAddressList_Len; // auto-gen: for list
  struct portsecuritypb_SecureEntry **SecureAddressList;
  enum portsecuritypb_PortSecurityViolationTypeOptions ViolationMode;
};

struct portsecuritypb_SecureEntry {
  // index : vlan_i_d + m_a_c_address
  long int VlanID;
  char *MACAddress;
};

struct portsecuritypb_PortSecureEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  struct portsecuritypb_SecureEntry *Entry;
};

// Status
struct portsecuritypb_Status {
  unsigned int List_Len; // auto-gen: for list
  struct portsecuritypb_PortSecurityStatusEntry **List;
};

struct portsecuritypb_PortSecurityStatusEntry {
  // Index (for update); unique. Only support INTERFACE_TYPE_PORT
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool Enabled;
  enum portsecuritypb_PortSecurityStatusTypeOptions Status;
  enum portsecuritypb_PortSecurityViolationTypeOptions ViolationMode;
  long int MaxMACCount;
  long int SecureMACCount;
  char *LastSourceMACaddressVlan;
};

// Internal :
struct portsecuritypb_PortSecurityAge {
  bool StaticAging;
  enum portsecuritypb_PortSecurityAgeModeTypeOptions Type;
  long int Time;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_portsecurity_portsecurity
