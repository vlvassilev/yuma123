
#ifndef _H_intri_pb_github_com_Intrising_intri_type_config_config
#define _H_intri_pb_github_com_Intrising_intri_type_config_config

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../github.com/Intrising/intri-type/common/common.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/access/access.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/acl/acl.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/cdp/cdp.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/dhcp/dhcp.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/dhcpserver/dhcpserver.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/fdb/fdb.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/files/files.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/gvrp/gvrp.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/isolation/isolation.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/lacp/lacp.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/lldp/lldp.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/loop/loop.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/mirroring/mirroring.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/monitor/monitor.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/multicast/multicast.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/network/network.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/poe/poe.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/port/port.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/portauthentication/portauthentication.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/portsecurity/portsecurity.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/ptp/ptp.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/qos/qos.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/sfp/sfp.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/stormcontrol/stormcontrol.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/stp/stp.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/system/system.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/time/time.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/timerange/timerange.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/timesync/timesync.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/udld/udld.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/userinterface/userinterface.pb.h"
#include "../../../../github.com/Intrising/intri-type/core/vlan/vlan.pb.h"
#include "../../../../github.com/Intrising/intri-type/log/log.pb.h"
#include "../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum configpb_StorageTypeOptions {
  // [In RAM Disk] Configuration is saved in RAM dist
  configpb_StorageTypeOptions_STORAGE_TYPE_RAM_DISK = 0,
};

enum configpb_SaveModeTypeOptions {
  // [Temporarily] Modification on config is temporarily before save.
  configpb_SaveModeTypeOptions_SAVE_MODE_TYPE_TEMPORARILY = 0,
};

enum configpb_ConfigTypeOptions {
  // [Running] Running Configuration
  configpb_ConfigTypeOptions_CONIFG_TYPE_RUNNING = 0,
  // [Default] Default Configuration
  configpb_ConfigTypeOptions_CONIFG_TYPE_DEFAULT = 1,
  // [Save] Save Configuration
  configpb_ConfigTypeOptions_CONIFG_TYPE_SAVE = 2,
};

enum configpb_ConfigStateTypeOptions {
  // [Saved] Configuration is saved
  configpb_ConfigStateTypeOptions_CONFIG_STATE_TYPE_SAVED = 0,
  // [Changed] Configuration is changed
  configpb_ConfigStateTypeOptions_CONFIG_STATE_TYPE_CHANGED = 1,
};

enum configpb_FactoryDefaultModeTypeOptions {
  // [Keep All] Reset default but keep user/network configs
  configpb_FactoryDefaultModeTypeOptions_FACTORY_DEFAULT_MODE_TYPE_KEEP_ALL = 0,
  // [Keep User Accounts] Reset default but keep user accounts
  configpb_FactoryDefaultModeTypeOptions_FACTORY_DEFAULT_MODE_TYPE_KEEP_USER_ACCOUNTS = 1,
  // [Keep Network Configs] Reset default but keep network configs
  configpb_FactoryDefaultModeTypeOptions_FACTORY_DEFAULT_MODE_TYPE_KEEP_NETWORK_CONFIGS = 2,
  // [Reset All] Reset to factory default
  configpb_FactoryDefaultModeTypeOptions_FACTORY_DEFAULT_MODE_TYPE_RESET_ALL = 3,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct configpb_ImportAction {
  char *FileURL;
  bool RebootAfterAction;
};

struct configpb_ExportAction {
  char *FileURL;
  bool IsFTPS;
  bool ForceTFTP;
};

struct configpb_SaveModeStatus {
  enum configpb_StorageTypeOptions SaveStorageOption;
  enum configpb_SaveModeTypeOptions SaveModeOption;
  enum configpb_ConfigStateTypeOptions ConfigStateOption;
  // google.protobuf.Timestamp config_last_updated = 4 [deprecated = true];
  char *ConfigLastUpdated;
};

struct configpb_RestoreDefaultType {
  enum configpb_FactoryDefaultModeTypeOptions Type;
};

struct configpb_AllServicesConfig {
  struct aclpb_Config *Acl;
  struct vlanpb_Config *Vlan;
  struct gvrppb_Config *Gvrp;
  struct accesspb_Config *Access;
  struct userinterfacepb_Config *UserInterface;
  struct systempb_Config *System;
  struct networkpb_Config *Network;
  struct timepb_Config *Time;
  struct portpb_Config *Port;
  struct sfppb_Config *Sfp;
  struct mirroringpb_Config *Mirroring;
  struct isolationpb_Config *Isolation;
  struct fdbpb_Config *Fdb;
  struct portsecuritypb_Config *PortSecurity;
  struct filespb_Config *Files;
  struct lacppb_Config *Lacp;
  struct multicastpb_Config *Multicast;
  struct stormcontrolpb_Config *StormControl;
  struct dhcppb_Config *Dhcp;
  struct qospb_Config *QoS;
  struct looppb_Config *Loop;
  struct lldppb_Config *Lldp;
  struct poepb_Config *Poe;
  struct cdppb_Config *Cdp;
  struct dhcpserverpb_Config *DhcpServer;
  struct stppb_STPConfig *Stp;
  struct stppb_MSTPConfig *Mstp;
  struct timerangepb_Config *TimeRange;
  struct dhcppb_ARPInspectionConfig *ArpInspection;
  struct ptppb_Config *Ptp;
  struct monitorpb_Config *Monitor;
  struct timesyncpb_Config *TimeSync;
  struct udldpb_Config *Udld;
  struct portauthenticationpb_Config *PortAuthentication;
  struct logpb_Config *Log;
};

// ValidateConfigResult: example below
struct configpb_ValidateConfigResult {
  // error struct field in list
  unsigned int List_Len; // auto-gen: for list
  char **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_config_config
