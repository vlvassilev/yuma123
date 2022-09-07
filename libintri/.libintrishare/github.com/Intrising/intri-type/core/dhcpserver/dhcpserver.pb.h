
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_dhcpserver_dhcpserver
#define _H_intri_pb_github_com_Intrising_intri_type_core_dhcpserver_dhcpserver

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/device/device.pb.h"
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

// internal The enum should not open to the user
enum dhcpserverpb_PoolTypeOptions {
  dhcpserverpb_PoolTypeOptions_POOL_TYPE_BASIC = 0,
  dhcpserverpb_PoolTypeOptions_POOL_TYPE_MAC_BASED = 1,
  dhcpserverpb_PoolTypeOptions_POOL_TYPE_PORT_BASED = 2,
};

enum dhcpserverpb_InterfaceType {
  // [Port] Select a port as the interface
  dhcpserverpb_InterfaceType_INTERFACE_TYPE_PORT = 0,
  // [VLAN] Select a VLAN as the interface
  dhcpserverpb_InterfaceType_INTERFACE_TYPE_VLAN = 1,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct dhcpserverpb_Config {
  struct dhcpserverpb_V4Config *V4;
};

struct dhcpserverpb_V4Config {
  struct dhcpserverpb_System *Sys;
  struct dhcpserverpb_Pool *Pool;
};

struct dhcpserverpb_System {
  bool Enabled;
};

struct dhcpserverpb_Pool {
  // There will be a basic pool in default config
  unsigned int List_Len; // auto-gen: for list
  struct dhcpserverpb_PoolEntry **List;
};

struct dhcpserverpb_PoolEntry {
  // The field is for furture use, don't need to be visible on GUI for now
  char *Name;
  // This field is temperarily for internal use
  struct devicepb_InterfaceIdentify *Interface;
  // 每一個 PoolEntry 都會有自己的 entry_type。
// 如果 type 是 basic,      就看 basic 這個欄位的 config
// 如果 type 是 mac_based,  就使用 mac_based 這個欄位
// 如果 type 是 port_based, 就使用 port_based 這個欄位
  enum dhcpserverpb_PoolTypeOptions EntryType;
  struct dhcpserverpb_Basic *Basic;
  struct dhcpserverpb_MACBased *MACBased;
  struct dhcpserverpb_PortBased *PortBased;
  // id 底層會自己產生，add config 的時候前端這個欄位填空字串就好，但是修改或刪除的時候要待在這個欄位裡，不能是空字串。
// basic pool 的 ID 固定為 'basic', default config 就會產生
// port based pool 的 ID 固定為 'port${portNo}' ，default config 就會產生
// mac based pool 的 ID 固定為傳下來的 macAddress, 在 add 的時候底層會自動加上去，所以這個欄位也填空字串就好
  char *ID;
};

struct dhcpserverpb_Basic {
  // The range low should be in the device's current IP's subnet
  char *RangeLow;
  // The range high should be in the device's current IP's subnet
  char *RangeHigh;
  // There is no limit for this field
  char *Netmask;
  // There is no limit for this field
  char *Gateway;
  char *PrimaryDNS;
  char *SecondaryDNS;
  // 120-86400
  int32_t LeaseTime;
};

struct dhcpserverpb_MACBased {
  // The macAddrress can not be duplicated with other PoolEntry
  char *MACAddress;
  char *DesiredIP;
};

struct dhcpserverpb_PortBased {
  // portNo should be pre-generated, no need to modify this field , [(validate.rules).int32 = {gte: 1, lte: 30}]
  int32_t PortNo;
  // Server won't assign the IP to this port if ignore is true
  bool Ignore;
  // Server should assign basic IP to this port if the desiredIP is ""
  char *DesiredIP;
};

struct dhcpserverpb_StatusEntry {
  // internal use, not required to be visible on UI
  char *Name;
  // internal use, not required to be visible on UI
  struct devicepb_InterfaceIdentify *Interface;
  int32_t PortNo;
  char *MACAddress;
  char *IPAddress;
  int32_t AvailableLeaseTime;
};

struct dhcpserverpb_Status {
  unsigned int List_Len; // auto-gen: for list
  struct dhcpserverpb_StatusEntry **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_dhcpserver_dhcpserver
