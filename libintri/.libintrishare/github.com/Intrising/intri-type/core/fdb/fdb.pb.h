
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_fdb_fdb
#define _H_intri_pb_github_com_Intrising_intri_type_core_fdb_fdb

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../../github.com/Intrising/intri-type/event/event.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum fdbpb_MACTableStateTypeOptions {
  // [Unused] The field was not set
  fdbpb_MACTableStateTypeOptions_MAC_TABLE_STATE_TYPE_UNUSED = 0,
  // [Other] Unspecified entry state
  fdbpb_MACTableStateTypeOptions_MAC_TABLE_STATE_TYPE_OTHER = 1,
  // [Invalid] This entry is not longer valid (aged)
  fdbpb_MACTableStateTypeOptions_MAC_TABLE_STATE_TYPE_INVALID = 2,
  // [Learned] This entry was learned and is valid
  fdbpb_MACTableStateTypeOptions_MAC_TABLE_STATE_TYPE_LEARNED = 3,
  // [Self] This entry corresponds to the local MAC address
  fdbpb_MACTableStateTypeOptions_MAC_TABLE_STATE_TYPE_SELF = 4,
  // [PACC] This entry was created by port access control
  fdbpb_MACTableStateTypeOptions_MAC_TABLE_STATE_TYPE_PACC = 5,
  // * This is an internal multicast address
  fdbpb_MACTableStateTypeOptions_MAC_TABLE_STATE_TYPE_MULTICAST = 6,
  // [Reject] This entry was rejected because of dropping
  fdbpb_MACTableStateTypeOptions_MAC_TABLE_STATE_TYPE_REJECT = 7,
};

enum fdbpb_PortOccupiedTypeOptions {
  // [None]
  fdbpb_PortOccupiedTypeOptions_PORT_OCCUPIED_TYPE_NONE = 0,
  // [PACC]
  fdbpb_PortOccupiedTypeOptions_PORT_OCCUPIED_TYPE_PACC = 1,
  // [Port Security]
  fdbpb_PortOccupiedTypeOptions_PORT_OCCUPIED_TYPE_PORT_SECURITY = 2,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct fdbpb_Config {
  struct fdbpb_AgingTime *AgeTime;
  struct fdbpb_PortLearningLimit *PortLimit;
  struct fdbpb_ForwardConfig *ForwardConfig;
  struct fdbpb_DropConfig *DropConfig;
};

struct fdbpb_AgingTime {
  // The unit of this field is second
  long int GlobalAgingTime;
};

struct fdbpb_ForwardConfig {
  // boundary is according to device.GetBoundary().FDB.ForwardLimit
  unsigned int List_Len; // auto-gen: for list
  struct fdbpb_ForwardConfigEntry **List;
};

struct fdbpb_ForwardConfigEntry {
  // m_a_c_address+vlan_i_d is unique Index (for add/remove/update)
  char *MACAddress;
  long int VlanID;
  // Only support INTERFACE_TYPE_PORT and INTERFACE_TYPE_TRUNK
  struct devicepb_InterfaceIdentify *IdentifyNo;
};

struct fdbpb_DropConfig {
  // boundary is according to device.GetBoundary().FDB.DropLimit
  unsigned int List_Len; // auto-gen: for list
  struct fdbpb_DropConfigEntry **List;
};

struct fdbpb_DropConfigEntry {
  // m_a_c_address+vlan_i_d is unique Index (for add/remove/update)
  char *MACAddress;
  long int VlanID;
  // Only support INTERFACE_TYPE_PORT and INTERFACE_TYPE_TRUNK
  struct devicepb_InterfaceIdentify *IdentifyNo;
};

struct fdbpb_PortLearningLimit {
  // boundary is according to device.GetBoundary().FDB.PortLearningLimit
  unsigned int List_Len; // auto-gen: for list
  struct fdbpb_PortLearningLimitEntry **List;
};

struct fdbpb_PortLearningLimitEntry {
  // Index (for update); unique. support INTERFACE_TYPE_PORT and INTERFACE_TYPE_TRUNK
  struct devicepb_InterfaceIdentify *IdentifyNo;
  long int Limit;
};

// Info :
struct fdbpb_Info {
  long int UsedAgingTime;
  long int NumberOfFreeEntries;
  long int NumberOfUsedEntries;
  long int NumberOfMacUnicastDynamicEntries;
  long int NumberOfMacUnicastStaticEntries;
  long int NumberOfMacMulticastDynamicEntries;
  long int NumberOfMacMulticastStaticEntries;
  long int NumberOfIpv4MulticastEntries;
  long int NumberOfIpv6MulticastEntries;
};

struct fdbpb_Status {
  // boundary is according to device.GetBoundary().FDB.FDBSize
  unsigned int List_Len; // auto-gen: for list
  struct fdbpb_StatusEntry **List;
};

// Index : m_a_c_address,identify_no,vlan_i_d; unique
struct fdbpb_StatusEntry {
  long int VlanID;
  char *MACAddress;
  // support INTERFACE_TYPE_PORT and INTERFACE_TYPE_TRUNK
  struct devicepb_InterfaceIdentify *IdentifyNo;
  enum fdbpb_MACTableStateTypeOptions State;
};

struct fdbpb_SpecificMac {
  char *MACAddress;
};

// Flush
struct fdbpb_FlushOption {
  bool DynamicAll;
  struct devicepb_PortList *Identify;
};

// internal use
struct fdbpb_PortOccupied {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  enum fdbpb_PortOccupiedTypeOptions Type;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_fdb_fdb
