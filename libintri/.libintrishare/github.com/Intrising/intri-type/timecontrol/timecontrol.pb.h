
#ifndef _H_intri_pb_github_com_Intrising_intri_type_timecontrol_timecontrol
#define _H_intri_pb_github_com_Intrising_intri_type_timecontrol_timecontrol

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include "../../../../github.com/golang/protobuf/ptypes/timestamp/timestamp.pb.h"
#include <stdbool.h>
#include <stdint.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum timecontrolpb_StatusInputBitTypeOptions {
  // [Los Live] Status input bit type los live
  timecontrolpb_StatusInputBitTypeOptions_STATUS_INPUT_BIT_TYPE_LOS_LIVE = 0,
  // [No Activity Live] Status input bit type no activity live
  timecontrolpb_StatusInputBitTypeOptions_STATUS_INPUT_BIT_TYPE_NO_ACTIVITY_LIVE = 1,
  // [Req Offs Lim Live] Status input bit type req offs lim live
  timecontrolpb_StatusInputBitTypeOptions_STATUS_INPUT_BIT_TYPE_REQ_OFFS_LIM_LIVE = 2,
  // [Trans Detect Live] Status input bit type trans detect live
  timecontrolpb_StatusInputBitTypeOptions_STATUS_INPUT_BIT_TYPE_TRANS_DETECT_LIVE = 3,
};

enum timecontrolpb_DPLLManageIndexTypeOptions {
  // [Index 0] Manage index 0
  timecontrolpb_DPLLManageIndexTypeOptions_DPLL_MANAGE_INDEX_0 = 0,
  // [Index 1] Manage index 1
  timecontrolpb_DPLLManageIndexTypeOptions_DPLL_MANAGE_INDEX_1 = 1,
  // [Index 2] Manage index 2
  timecontrolpb_DPLLManageIndexTypeOptions_DPLL_MANAGE_INDEX_2 = 2,
  // [Index 3] Manage index 3
  timecontrolpb_DPLLManageIndexTypeOptions_DPLL_MANAGE_INDEX_3 = 3,
  // [Index 4] Manage index 4
  timecontrolpb_DPLLManageIndexTypeOptions_DPLL_MANAGE_INDEX_4 = 4,
  // [Index 5] Manage index 5
  timecontrolpb_DPLLManageIndexTypeOptions_DPLL_MANAGE_INDEX_5 = 5,
  // [Index 6] Manage index 6
  timecontrolpb_DPLLManageIndexTypeOptions_DPLL_MANAGE_INDEX_6 = 6,
  // [Index 7] Manage index 7
  timecontrolpb_DPLLManageIndexTypeOptions_DPLL_MANAGE_INDEX_7 = 7,
};

enum timecontrolpb_DPLLStatusBitTypeOptions {
  // [Freerun] Freerun
  timecontrolpb_DPLLStatusBitTypeOptions_DPLL_STATUS_BIT_TYPE_FREERUN = 0,
  // [Lockacq] Lockacq
  timecontrolpb_DPLLStatusBitTypeOptions_DPLL_STATUS_BIT_TYPE_LOCKACQ = 1,
  // [Lockrec] Lockrec
  timecontrolpb_DPLLStatusBitTypeOptions_DPLL_STATUS_BIT_TYPE_LOCKREC = 2,
  // [Locked] Locked
  timecontrolpb_DPLLStatusBitTypeOptions_DPLL_STATUS_BIT_TYPE_LOCKED = 3,
  // [Holdover] Holdover
  timecontrolpb_DPLLStatusBitTypeOptions_DPLL_STATUS_BIT_TYPE_HOLDOVER = 4,
  // [Openloop] Openloop
  timecontrolpb_DPLLStatusBitTypeOptions_DPLL_STATUS_BIT_TYPE_OPENLOOP = 5,
  // [Disabled] Disabled
  timecontrolpb_DPLLStatusBitTypeOptions_DPLL_STATUS_BIT_TYPE_DISABLED = 6,
};

enum timecontrolpb_DPLLRefPriorityBitTypeOptions {
  // [Priority 0] Dpll ref priority bit type 0
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_0 = 0,
  // [Priority 1] Dpll ref priority bit type 1
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_1 = 1,
  // [Priority 2] Dpll ref priority bit type 2
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_2 = 2,
  // [Priority 3] Dpll ref priority bit type 3
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_3 = 3,
  // [Priority 4] Dpll ref priority bit type 4
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_4 = 4,
  // [Priority 5] Dpll ref priority bit type 5
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_5 = 5,
  // [Priority 6] Dpll ref priority bit type 6
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_6 = 6,
  // [Priority 7] Dpll ref priority bit type 7
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_7 = 7,
  // [Priority 8] Dpll ref priority bit type 8
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_8 = 8,
  // [Priority 9] Dpll ref priority bit type 9
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_9 = 9,
  // [Priority 10] Dpll ref priority bit type 10
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_10 = 10,
  // [Priority 11] Dpll ref priority bit type 11
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_11 = 11,
  // [Priority 12] Dpll ref priority bit type 12
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_12 = 12,
  // [Priority 13] Dpll ref priority bit type 13
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_13 = 13,
  // [Priority 14] Dpll ref priority bit type 14
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_14 = 14,
  // [Priority 15] Dpll ref priority bit type 15
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_15 = 15,
  // [Priority 16] Dpll ref priority bit type 16
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_16 = 16,
  // [Priority 17] Dpll ref priority bit type 17
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_17 = 17,
  // [Priority 18] Dpll ref priority bit type 18
  timecontrolpb_DPLLRefPriorityBitTypeOptions_DPLL_REF_PRIORITY_BIT_TYPE_18 = 18,
};

enum timecontrolpb_ToDSourceTypeOptions {
  // [SYSTEM] Source system
  timecontrolpb_ToDSourceTypeOptions_SYSTEM = 0,
  // [TIME_CONTROL] Source time_control
  timecontrolpb_ToDSourceTypeOptions_TIME_CONTROL = 1,
  // [RTC] Source rtc
  timecontrolpb_ToDSourceTypeOptions_RTC = 2,
  // [GPS] Source gps
  timecontrolpb_ToDSourceTypeOptions_GPS = 3,
};

enum timecontrolpb_DPLLToDTypeOptions {
  // [ToD 0] ToD 0
  timecontrolpb_DPLLToDTypeOptions_DPLL_MANAGE_TOD_0 = 0,
  // [ToD 1] ToD 1
  timecontrolpb_DPLLToDTypeOptions_DPLL_MANAGE_TOD_1 = 1,
  // [ToD 2] ToD 2
  timecontrolpb_DPLLToDTypeOptions_DPLL_MANAGE_TOD_2 = 2,
  // [ToD 3] ToD 3
  timecontrolpb_DPLLToDTypeOptions_DPLL_MANAGE_TOD_3 = 3,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct timecontrolpb_InputClockManage {
  unsigned int List_Len; // auto-gen: for list
  struct timecontrolpb_InputClockManageEntry **List;
};

struct timecontrolpb_InputClockManageEntry {
  enum devicepb_InputClockTypeOptions InputCLK;
};

struct timecontrolpb_MonitorStatus {
  unsigned int List_Len; // auto-gen: for list
  struct timecontrolpb_MonitorStatusEntry **List;
};

struct timecontrolpb_MonitorStatusEntry {
  enum devicepb_InputClockTypeOptions InputCLK;
  unsigned int StatusList_Len; // auto-gen: for list
  struct timecontrolpb_StatusInputBit **StatusList;
  bool Valid;
};

struct timecontrolpb_StatusInputBit {
  enum timecontrolpb_StatusInputBitTypeOptions Type;
  bool Valid;
};

struct timecontrolpb_DPLLManage {
  unsigned int List_Len; // auto-gen: for list
  struct timecontrolpb_DPLLManageEntry **List;
};

struct timecontrolpb_DPLLManageEntry {
  enum timecontrolpb_DPLLManageIndexTypeOptions Index;
};

struct timecontrolpb_DPLLStatus {
  unsigned int List_Len; // auto-gen: for list
  struct timecontrolpb_DPLLStatusEntry **List;
};

struct timecontrolpb_DPLLStatusEntry {
  enum timecontrolpb_DPLLManageIndexTypeOptions Index;
  enum timecontrolpb_DPLLStatusBitTypeOptions StatusType;
  enum devicepb_InputClockTypeOptions InputCLK;
};

struct timecontrolpb_DPLLRefPriorityManage {
  unsigned int List_Len; // auto-gen: for list
  struct timecontrolpb_DPLLRefPriorityManageEntry **List;
};

struct timecontrolpb_DPLLRefPriorityManageEntry {
  enum timecontrolpb_DPLLManageIndexTypeOptions Index;
  unsigned int PriorityList_Len; // auto-gen: for list
  enum timecontrolpb_DPLLRefPriorityBitTypeOptions *PriorityList;
};

struct timecontrolpb_DPLLRefPriority {
  unsigned int List_Len; // auto-gen: for list
  struct timecontrolpb_DPLLRefPriorityEntry **List;
};

struct timecontrolpb_DPLLRefPriorityEntry {
  enum timecontrolpb_DPLLManageIndexTypeOptions Index;
  unsigned int PrioritList_Len; // auto-gen: for list
  struct timecontrolpb_PriorityList **PrioritList;
};

struct timecontrolpb_PriorityList {
  enum timecontrolpb_DPLLRefPriorityBitTypeOptions Priority;
  enum devicepb_InputClockTypeOptions InputCLK;
};

struct timecontrolpb_SyncEInputClockSpeed {
  unsigned int List_Len; // auto-gen: for list
  struct timecontrolpb_SyncEInputClockSpeedEntry **List;
};

struct timecontrolpb_SyncEInputClockSpeedEntry {
  enum devicepb_InputClockTypeOptions InputCLK;
  enum devicepb_PortPropertyTypeOptions Speed;
};

struct timecontrolpb_ToDSource {
  bool Enable;
  enum timecontrolpb_ToDSourceTypeOptions Source;
};

struct timecontrolpb_DPLLToDManage {
  unsigned int List_Len; // auto-gen: for list
  struct timecontrolpb_DPLLToDManageEntry **List;
};

struct timecontrolpb_DPLLToDManageEntry {
  enum timecontrolpb_DPLLToDTypeOptions Index;
};

struct timecontrolpb_ToDTime {
  unsigned int List_Len; // auto-gen: for list
  struct timecontrolpb_ToDTimeEntry **List;
};

struct timecontrolpb_ToDTimeEntry {
  enum timecontrolpb_DPLLToDTypeOptions Index;
  struct timestamppb_Timestamp *DateTime;
};

struct timecontrolpb_DPLLFrequencyControl {
  unsigned int List_Len; // auto-gen: for list
  struct timecontrolpb_DPLLFrequencyControlEntry **List;
};

struct timecontrolpb_DPLLFrequencyControlEntry {
  enum timecontrolpb_DPLLManageIndexTypeOptions Index;
  int64_t FreqOffset;
};

struct timecontrolpb_DPLLPhaseControl {
  unsigned int List_Len; // auto-gen: for list
  struct timecontrolpb_DPLLPhaseControlEntry **List;
};

struct timecontrolpb_DPLLPhaseControlEntry {
  enum timecontrolpb_DPLLManageIndexTypeOptions Index;
  int32_t PhOffset;
};

struct timecontrolpb_DPLLPhaseSlopeLimit {
  unsigned int List_Len; // auto-gen: for list
  struct timecontrolpb_DPLLPhaseSlopeLimitEntry **List;
};

struct timecontrolpb_DPLLPhaseSlopeLimitEntry {
  enum timecontrolpb_DPLLManageIndexTypeOptions Index;
  int32_t Limit;
};

struct timecontrolpb_DPLLPhaseControlTimer {
  unsigned int List_Len; // auto-gen: for list
  struct timecontrolpb_DPLLPhaseControlTimerEntry **List;
};

struct timecontrolpb_DPLLPhaseControlTimerEntry {
  enum timecontrolpb_DPLLManageIndexTypeOptions Index;
  int32_t Timeout;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_timecontrol_timecontrol
