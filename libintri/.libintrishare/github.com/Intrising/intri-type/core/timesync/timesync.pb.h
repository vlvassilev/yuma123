
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_timesync_timesync
#define _H_intri_pb_github_com_Intrising_intri_type_core_timesync_timesync

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

enum timesyncpb_SyncEModeTypeOptions {
  // [Disabled]
  timesyncpb_SyncEModeTypeOptions_SYNCE_MODE_TYPE_DISABLED = 0,
  // [Static]
  timesyncpb_SyncEModeTypeOptions_SYNCE_MODE_TYPE_STATIC = 1,
  // [ESMC]
  timesyncpb_SyncEModeTypeOptions_SYNCE_MODE_TYPE_ESMC = 2,
};

enum timesyncpb_SyncSourceTypeOptions {
  // [GNSS]
  timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_GNSS = 0,
  // [SyncE]
  timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_SYNCE = 1,
  // [10MHz Input]
  timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_10MHZ_INPUT = 2,
  // [1PPS Input]
  timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_1PPS_INPUT = 3,
  // [BITS Input]
  timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_BITS_INPUT = 4,
  // [PTP]
  timesyncpb_SyncSourceTypeOptions_SYNC_SOURCE_TYPE_PTP = 5,
};

enum timesyncpb_LockStatusTypeOptions {
  // [FreeRun]
  timesyncpb_LockStatusTypeOptions_LOCK_STATUS_TYPE_FREERUN = 0,
  // [Lock Acquisition]
  timesyncpb_LockStatusTypeOptions_LOCK_STATUS_TYPE_LOCK_ACQUISITION = 1,
  // [Locked]
  timesyncpb_LockStatusTypeOptions_LOCK_STATUS_TYPE_LOCKED = 2,
  // [Holdover]
  timesyncpb_LockStatusTypeOptions_LOCK_STATUS_TYPE_HOLDOVER = 3,
};

enum timesyncpb_SignalTypeOptions {
  // [OK]
  timesyncpb_SignalTypeOptions_SIGNAL_TYPE_OK = 0,
  // [Loss]
  timesyncpb_SignalTypeOptions_SIGNAL_TYPE_LOSS = 1,
};

enum timesyncpb_ToDMessageTypeOptions {
  // [NMEA GPZDA]
  timesyncpb_ToDMessageTypeOptions_TOD_MESSAGE_TYPE_NMEA_GPZDA = 0,
};

enum timesyncpb_GNSSStateTypeOptions {
  // [Disable]
  timesyncpb_GNSSStateTypeOptions_GNSS_STATE_TYPE_DISABLE = 0,
  // [Sync]
  timesyncpb_GNSSStateTypeOptions_GNSS_STATE_TYPE_SYNC = 1,
  // [Tracking]
  timesyncpb_GNSSStateTypeOptions_GNSS_STATE_TYPE_TRACKING = 2,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

// Config
struct timesyncpb_Config {
  struct timesyncpb_GNSSConfig *GNSS;
  struct timesyncpb_SyncEConfig *SyncE;
  struct timesyncpb_SyncSourceConfig *SyncSource;
  struct timesyncpb_ReferenceOutput *Reference;
};

struct timesyncpb_GNSSConfig {
  bool Enabled;
};

struct timesyncpb_SyncEConfig {
  enum timesyncpb_SyncEModeTypeOptions Mode;
  struct devicepb_InterfaceIdentify *ReferenceIdentifyNo;
};

struct timesyncpb_SyncSourceConfig {
  enum timesyncpb_SyncSourceTypeOptions Source;
};

struct timesyncpb_ReferenceOutput {
  struct timesyncpb_ToDConfig *Tod;
};

struct timesyncpb_ToDConfig {
  bool Enabled;
  enum timesyncpb_ToDMessageTypeOptions MessageType;
};

// Status
struct timesyncpb_GNSStatus {
  enum timesyncpb_GNSSStateTypeOptions State;
  char *Longitude;
  char *Latitude;
  char *DateTime;
};

struct timesyncpb_SyncEStatus {
  struct devicepb_InterfaceIdentify *ReferenceIdentifyNo;
  enum timesyncpb_SignalTypeOptions Signal;
  enum timesyncpb_LockStatusTypeOptions LockStatus;
};

struct timesyncpb_SyncSourceStatus {
  enum timesyncpb_LockStatusTypeOptions LockStatus;
  unsigned int List_Len; // auto-gen: for list
  struct timesyncpb_SyncSourceInputStatusEntry **List;
};

struct timesyncpb_SyncSourceInputStatusEntry {
  enum timesyncpb_SyncSourceTypeOptions Source;
  enum timesyncpb_SignalTypeOptions Signal;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_timesync_timesync
