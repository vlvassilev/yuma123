
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_port_port
#define _H_intri_pb_github_com_Intrising_intri_type_core_port_port

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/core/stp/stp.pb.h"
#include "../../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../../github.com/Intrising/intri-type/event/event.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum portpb_LinkStateTypeOptions {
  // [Link Down] Link is not established. No communication. Ethernet LED is off
  portpb_LinkStateTypeOptions_LINK_STATE_TYPE_LINK_DOWN = 0,
  // [Blocking] Port is blocked. No communication. Ethernet LED indicates yellow.
  portpb_LinkStateTypeOptions_LINK_STATE_TYPE_BLOCKING = 1,
  // [Learning] Port is learning MAC addresses. No communication. Ethernet LED indicates yellow.
  portpb_LinkStateTypeOptions_LINK_STATE_TYPE_LEARNING = 2,
  // [Forwarding] Port is forwarding data. Ethernet LED indicates green.
  portpb_LinkStateTypeOptions_LINK_STATE_TYPE_FORWARDING = 3,
  // [Unauth VLAN] Port is forwarding data on the unauthorized_vlan only. Ethernet LED indicates green.
  portpb_LinkStateTypeOptions_LINK_STATE_TYPE_UNAUTH_VLAN = 4,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

// Config for `Port` 
struct portpb_ConfigEntry {
  // Only support INTERFACE_TYPE_PORT
  struct devicepb_InterfaceIdentify *IdentifyNo;
  char *Alias;
  bool PortOperation;
  enum eventpb_PortSpeedDuplexTypeOptions SpeedDuplex;
  bool FlowControl;
  bool EnergyEfficiency;
};

struct portpb_Config {
  unsigned int List_Len; // auto-gen: for list
  struct portpb_ConfigEntry **List;
};

struct portpb_AliasEntry {
  // Only support INTERFACE_TYPE_PORT
  struct devicepb_InterfaceIdentify *IdentifyNo;
  char *Alias;
};

struct portpb_AliasConfig {
  unsigned int List_Len; // auto-gen: for list
  struct portpb_AliasEntry **List;
};

struct portpb_OperationEntry {
  // Only support INTERFACE_TYPE_PORT
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool PortOperation;
};

struct portpb_OperationConfig {
  unsigned int List_Len; // auto-gen: for list
  struct portpb_OperationEntry **List;
};

struct portpb_SpeedDuplexEntry {
  // Only support INTERFACE_TYPE_PORT
  struct devicepb_InterfaceIdentify *IdentifyNo;
  enum eventpb_PortSpeedDuplexTypeOptions SpeedDuplex;
};

struct portpb_SpeedDuplexConfig {
  unsigned int List_Len; // auto-gen: for list
  struct portpb_SpeedDuplexEntry **List;
};

struct portpb_FlowControlEntry {
  // Only support INTERFACE_TYPE_PORT
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool FlowControl;
};

struct portpb_FlowControlConfig {
  unsigned int List_Len; // auto-gen: for list
  struct portpb_FlowControlEntry **List;
};

struct portpb_EnergyEfficiencyEntry {
  // Only support INTERFACE_TYPE_PORT
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool EnergyEfficiency;
};

struct portpb_EnergyEfficiencyConfig {
  unsigned int List_Len; // auto-gen: for list
  struct portpb_EnergyEfficiencyEntry **List;
};

struct portpb_StatusEntry {
  // Only support INTERFACE_TYPE_PORT
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool Enabled;
  bool LinkUp;
  char *LastLinkChange;
  enum eventpb_PortSpeedDuplexTypeOptions SpeedDuplexUsed;
  bool FlowControlUsed;
  bool EEEActive;
  enum stppb_PortStatusStateTypeOptions LinkState;
};

struct portpb_Status {
  unsigned int List_Len; // auto-gen: for list
  struct portpb_StatusEntry **List;
};

// This is the logical port status, not lag port
struct portpb_LgPortStatusEntry {
  // Physical/lag port both
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool LinkUp;
  // If existed is true means this LAG port is active, otherwise this LAG port is deactive
  bool Existed;
  // Physical port only for this field. If existed is true , active_no will exist, otherwise is nil
  struct devicepb_InterfaceIdentify *ActiveNo;
  // The item of this arrya is physical port only. This array will be empty if exist is false, otherwise the length of this array is at least 1
  unsigned int Members_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **Members;
};

// internal use
struct portpb_LgPortStatus {
  unsigned int List_Len; // auto-gen: for list
  struct portpb_LgPortStatusEntry **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_port_port
