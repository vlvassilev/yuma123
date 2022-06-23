
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_stp_stp
#define _H_intri_pb_github_com_Intrising_intri_type_core_stp_stp

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>
#include <stdint.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum stppb_BridgeConfigModeTypeOptions {
  // [Disabled] Disabled mode
  stppb_BridgeConfigModeTypeOptions_BRIDGE_CONFIG_MODE_TYPE_DISABLED = 0,
  // [STP] Stp bridge mode
  stppb_BridgeConfigModeTypeOptions_BRIDGE_CONFIG_MODE_TYPE_STP = 1,
  // [RSTP] Rstp bridge mode
  stppb_BridgeConfigModeTypeOptions_BRIDGE_CONFIG_MODE_TYPE_RSTP = 2,
  // [MSTP] Mstp bridge mode
  stppb_BridgeConfigModeTypeOptions_BRIDGE_CONFIG_MODE_TYPE_MSTP = 3,
};

enum stppb_PortConfigAdminP2PPortTypeOptions {
  // [Auto] Auto mode
  stppb_PortConfigAdminP2PPortTypeOptions_PORT_CONFIG_ADMIN_P2P_PORT_TYPE_AUTO = 0,
  // [Force False] Force false mode
  stppb_PortConfigAdminP2PPortTypeOptions_PORT_CONFIG_ADMIN_P2P_PORT_TYPE_FORCE_FALSE = 1,
  // [Force True] Force true mode
  stppb_PortConfigAdminP2PPortTypeOptions_PORT_CONFIG_ADMIN_P2P_PORT_TYPE_FORCE_TRUE = 2,
};

enum stppb_PortConfigBPDUGuardTypeOptions {
  // [Disabled] Disabled mode
  stppb_PortConfigBPDUGuardTypeOptions_PORT_CONFIG_BPDU_GUARD_TYPE_DISABLED = 0,
  // [Drop And Event] Drop and event mode
  stppb_PortConfigBPDUGuardTypeOptions_PORT_CONFIG_BPDU_GUARD_TYPE_DROP_AND_EVENT = 1,
  // [Block Port] Block port mode
  stppb_PortConfigBPDUGuardTypeOptions_PORT_CONFIG_BPDU_GUARD_TYPE_BLOCK_PORT = 2,
};

enum stppb_PortStatusStateTypeOptions {
  // [Unknown] Unknown state
  stppb_PortStatusStateTypeOptions_PORT_STATUS_STATE_TYPE_UNKNOWN = 0,
  // [Discarding] Discarding state
  stppb_PortStatusStateTypeOptions_PORT_STATUS_STATE_TYPE_DISCARDING = 1,
  // [Learning] Learning state 
  stppb_PortStatusStateTypeOptions_PORT_STATUS_STATE_TYPE_LEARNING = 2,
  // [Forwarding] Forwarding state
  stppb_PortStatusStateTypeOptions_PORT_STATUS_STATE_TYPE_FORWARDING = 3,
  // [Blocking] Blocking state
  stppb_PortStatusStateTypeOptions_PORT_STATUS_STATE_TYPE_BLOCKING = 4,
  // [Listening] Listening state
  stppb_PortStatusStateTypeOptions_PORT_STATUS_STATE_TYPE_LISTENING = 5,
  // [Broken] Broken state
  stppb_PortStatusStateTypeOptions_PORT_STATUS_STATE_TYPE_BROKEN = 6,
};

enum stppb_PortStatusRoleTypeOptions {
  // [Unknown] Unknown
  stppb_PortStatusRoleTypeOptions_PORT_STATUS_ROLE_TYPE_UNKNOWN = 0,
  // [Root bridge] Root bridge
  stppb_PortStatusRoleTypeOptions_PORT_STATUS_ROLE_TYPE_ROOT = 1,
  // [Designated bridge] Designated bridge
  stppb_PortStatusRoleTypeOptions_PORT_STATUS_ROLE_TYPE_DESIGNATED = 2,
  // [Alternate bridge] Alternate bridge
  stppb_PortStatusRoleTypeOptions_PORT_STATUS_ROLE_TYPE_ALTERNATE = 3,
  // [Backup bridge] Backup bridge
  stppb_PortStatusRoleTypeOptions_PORT_STATUS_ROLE_TYPE_BACKUP = 4,
  // [Master bridge] Master bridge
  stppb_PortStatusRoleTypeOptions_PORT_STATUS_ROLE_TYPE_MASTER = 5,
  // [Disabled] Disabled
  stppb_PortStatusRoleTypeOptions_PORT_STATUS_ROLE_TYPE_DISABLED = 6,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct stppb_MSTPConfig {
  enum stppb_BridgeConfigModeTypeOptions Mode;
  char *Name;
  int32_t Revision;
  int32_t MaxAge;
  int32_t HelloTime;
  int32_t ForwardDelay;
  int32_t MaxHops;
  int32_t TxHoldCount;
  struct stppb_CISTEntry *Cist;
  unsigned int Mstis_Len; // auto-gen: for list
  struct stppb_MSTIEntry **Mstis;
};

struct stppb_CISTEntry {
  int32_t No;
  // in steps of 4096
  int32_t Priority;
  unsigned int Ports_Len; // auto-gen: for list
  struct stppb_MSTPCistPort **Ports;
};

struct stppb_MSTIEntry {
  // Index (for update/delete)
  int32_t No;
  // the `int32` in the field `vlans` should exist in VLAN filter list (`VLANConfig`, `filters`)
  unsigned int Vlans_Len; // auto-gen: for list
  int32_t *Vlans;
  bool Enabled;
  // in steps of 4096
  int32_t Priority;
  unsigned int Ports_Len; // auto-gen: for list
  struct stppb_MSTPMstiPort **Ports;
};

struct stppb_MSTIList {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_MSTIEntry **List;
};

struct stppb_MSTPCistPort {
  // Index (for update)
// - if `no` is physical port then the `no` value should less than 100
// - if `no` is lag port then the `no` value should grater than 100
  int32_t No;
  int32_t PathCost;
  int32_t Priority;
  bool STPEnabled;
  enum stppb_PortConfigAdminP2PPortTypeOptions P2PMode;
  bool EdgeMode;
  enum stppb_PortConfigBPDUGuardTypeOptions BPDUGuard;
  // Brian: the following are support on later phase
// bool bpdu_receive_only = 8;
// bool restrict_tcn = 9;
  bool RestrictRoot;
};

struct stppb_MSTPMstiPort {
  // Index (for update)
// - if `no` is physical port then the `no` value should less than 100
// - if `no` is lag port then the `no` value should grater than 100
  int32_t No;
  int32_t PathCost;
  int32_t Priority;
};

// Status
struct stppb_MSTPID {
  char *MACAddress;
  // in steps of 4096
  int32_t Priority;
};

struct stppb_CISTStatus {
  int32_t No;
  struct stppb_MSTPID *BridgeID;
  struct stppb_MSTPID *RootID;
  int32_t RootPort;
  int32_t RootCost;
  struct stppb_MSTPID *RegionalRoot;
  int32_t InternalRootCost;
  bool TopologyChange;
  int32_t TopologyChangeCount;
  int32_t TimeSinceTopologyChange;
  unsigned int Ports_Len; // auto-gen: for list
  struct stppb_CISTPortEntryStatus **Ports;
};

struct stppb_CISTPortEntryStatus {
  int32_t No;
  char *Role;
  char *State;
  int32_t Priority;
  int32_t PathCost;
  bool EdgeMode;
  bool P2PMode;
  int32_t Uptime;
  char *BPDUGuard;
};

struct stppb_MSTIStatus {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_MSTIStatusEntry **List;
};

struct stppb_MSTIStatusEntry {
  int32_t No;
  struct stppb_MSTPID *BridgeID;
  struct stppb_MSTPID *RootID;
  int32_t RootPort;
  int32_t RootCost;
  bool TopologyChange;
  int32_t TopologyChangeCount;
  int32_t TimeSinceTopologyChange;
  unsigned int Ports_Len; // auto-gen: for list
  struct stppb_MSTIPortStatusEntry **Ports;
};

struct stppb_MSTIPortStatusEntry {
  int32_t No;
  char *Role;
  char *State;
  int32_t Priority;
  int32_t PathCost;
  int32_t Uptime;
};

// MSTP-VLAN
struct stppb_MSTPVLANGroupPortEntry {
  int32_t PortNo;
  bool Tagged;
};

struct stppb_MSTPVLANGroupEntry {
  int32_t VLANID;
  unsigned int Ports_Len; // auto-gen: for list
  struct stppb_MSTPVLANGroupPortEntry **Ports;
};

// STP Config 
struct stppb_STPConfigBridge {
  enum stppb_BridgeConfigModeTypeOptions Mode;
  int32_t Priority;
  int32_t HelloTime;
  int32_t MaxAge;
  int32_t ForwardDelay;
  int32_t TxHoldCount;
  char *MSTPRegionName;
  int32_t MSTPRevisionLevel;
  int32_t MSTPMaxHops;
};

struct stppb_STPConfigPortEntry {
  // Ian:
// 1. if add => [(validate.rules).int32 = {in: [1, 2, 3, 4, 5, 6, 7, 101, 102, 103]}]
// 2. make proto will error below
// error:
// found "[" but expected [constant]:found "[" but expected [;]. Use -v for more details
// --protolint_out: protoc-gen-protolint: Plugin failed with status code 2.
  int32_t PortNo;
  bool Enabled;
  int32_t Priority;
  enum stppb_PortConfigAdminP2PPortTypeOptions AdminP2PPort;
  bool AdminEdgePort;
  int32_t AdminPathCost;
  int32_t MSTPDefaultPriority;
  unsigned int MSTPPortPriority_Len; // auto-gen: for list
  char **MSTPPortPriority;
  int32_t MSTPDefaultAdminPathCost;
  unsigned int MSTPPortAdminPathCost_Len; // auto-gen: for list
  char **MSTPPortAdminPathCost;
  enum stppb_PortConfigBPDUGuardTypeOptions BPDUGuard;
};

struct stppb_STPConfigMSTPGroupEntry {
  int32_t MSTPID;
  int32_t BridgePriority;
  unsigned int VIDs_Len; // auto-gen: for list
  int32_t *VIDs;
};

struct stppb_STPConfig {
  struct stppb_STPConfigBridge *Bridge;
  unsigned int Ports_Len; // auto-gen: for list
  struct stppb_STPConfigPortEntry **Ports;
  unsigned int MSTPGroups_Len; // auto-gen: for list
  struct stppb_STPConfigMSTPGroupEntry **MSTPGroups;
};

struct stppb_BridgeConfigMode {
  enum stppb_BridgeConfigModeTypeOptions Mode;
};

struct stppb_BridgeConfigPriority {
  int32_t Priority;
};

struct stppb_BridgeConfigHelloTime {
  int32_t HelloTime;
};

struct stppb_BridgeConfigMaxAge {
  int32_t MaxAge;
};

struct stppb_BridgeConfigForwardDelay {
  int32_t ForwardDelay;
};

struct stppb_BridgeConfigTxHoldCount {
  int32_t TxHoldCount;
};

struct stppb_BridgeConfigMSTPRegionName {
  char *MSTPRegionName;
};

struct stppb_BridgeConfigMSTPRevisionLevel {
  int32_t MSTPRevisionLevel;
};

struct stppb_BridgeConfigMSTPMaxHops {
  int32_t MSTPMaxHops;
};

struct stppb_STPPortConfigEnabledEntry {
  int32_t PortNo;
  bool Enabled;
};

struct stppb_STPPortConfigEnabled {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_STPPortConfigEnabledEntry **List;
};

struct stppb_STPPortConfigPriorityEntry {
  int32_t PortNo;
  int32_t Priority;
};

struct stppb_STPPortConfigPriority {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_STPPortConfigPriorityEntry **List;
};

struct stppb_STPPortConfigAdminP2PPortEntry {
  int32_t PortNo;
  enum stppb_PortConfigAdminP2PPortTypeOptions AdminP2PPort;
};

struct stppb_STPPortConfigAdminP2PPort {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_STPPortConfigAdminP2PPortEntry **List;
};

struct stppb_STPPortConfigAdminEdgePortEntry {
  int32_t PortNo;
  bool AdminEdgePort;
};

struct stppb_STPPortConfigAdminEdgePort {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_STPPortConfigAdminEdgePortEntry **List;
};

struct stppb_STPPortConfigAdminPathCostEntry {
  int32_t PortNo;
  int32_t AdminPathCost;
};

struct stppb_STPPortConfigAdminPathCost {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_STPPortConfigAdminPathCostEntry **List;
};

struct stppb_STPPortConfigMSTPDefaultPriorityEntry {
  int32_t PortNo;
  int32_t MSTPDefaultPriority;
};

struct stppb_STPPortConfigMSTPDefaultPriority {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_STPPortConfigMSTPDefaultPriorityEntry **List;
};

struct stppb_STPPortConfigMSTPPortPriorityEntry {
  int32_t PortNo;
  unsigned int MSTPPortPriority_Len; // auto-gen: for list
  char **MSTPPortPriority;
};

struct stppb_STPPortConfigMSTPPortPriority {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_STPPortConfigMSTPPortPriorityEntry **List;
};

struct stppb_STPPortConfigMSTPDefaultAdminPathCostEntry {
  int32_t PortNo;
  int32_t MSTPDefaultAdminPathCost;
};

struct stppb_STPPortConfigMSTPDefaultAdminPathCost {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_STPPortConfigMSTPDefaultAdminPathCostEntry **List;
};

struct stppb_STPPortConfigMSTPPortAdminPathCostEntry {
  int32_t PortNo;
  unsigned int MSTPPortAdminPathCost_Len; // auto-gen: for list
  char **MSTPPortAdminPathCost;
};

struct stppb_STPPortConfigMSTPPortAdminPathCost {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_STPPortConfigMSTPPortAdminPathCostEntry **List;
};

struct stppb_STPPortConfigBPDUGuardEntry {
  int32_t PortNo;
  enum stppb_PortConfigBPDUGuardTypeOptions BPDUGuard;
};

struct stppb_STPPortConfigBPDUGuard {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_STPPortConfigBPDUGuardEntry **List;
};

struct stppb_STPPortConfigBPDUReceiveOnlyEntry {
  int32_t PortNo;
  bool BPDUReceiveOnly;
};

struct stppb_STPPortConfigBPDUReceiveOnly {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_STPPortConfigBPDUReceiveOnlyEntry **List;
};

struct stppb_STPPortConfigRestrictTcnEntry {
  int32_t PortNo;
  bool RestrictTcn;
};

struct stppb_STPPortConfigRestrictTcn {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_STPPortConfigRestrictTcnEntry **List;
};

struct stppb_STPPortConfigRestrictRootEntry {
  int32_t PortNo;
  bool RestrictRoot;
};

struct stppb_STPPortConfigRestrictRoot {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_STPPortConfigRestrictRootEntry **List;
};

struct stppb_MSTPIDList {
  unsigned int IDList_Len; // auto-gen: for list
  int32_t *IDList;
};

struct stppb_STPMSTPGroupEntry {
  int32_t MSTPID;
  int32_t BridgePriority;
  unsigned int VIDs_Len; // auto-gen: for list
  int32_t *VIDs;
};

struct stppb_MSTPGroup {
  unsigned int List_Len; // auto-gen: for list
  struct stppb_STPMSTPGroupEntry **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_stp_stp