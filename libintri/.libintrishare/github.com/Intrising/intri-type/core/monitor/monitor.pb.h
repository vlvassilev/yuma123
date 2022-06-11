
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_monitor_monitor
#define _H_intri_pb_github_com_Intrising_intri_type_core_monitor_monitor

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../../github.com/Intrising/intri-type/event/event.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>
#include <stdint.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum monitorpb_DeviceMonitorTypeOptions {
  // [Temperature] cpu temperature
  monitorpb_DeviceMonitorTypeOptions_DEVICE_MONITOR_TYPE_TEMPERATURE = 0,
  // [Fan 1] fan 1 rpm
  monitorpb_DeviceMonitorTypeOptions_DEVICE_MONITOR_TYPE_FAN_1 = 1,
  // [Fan 2] fan 2 rpm
  monitorpb_DeviceMonitorTypeOptions_DEVICE_MONITOR_TYPE_FAN_2 = 2,
  // [Fan 3] fan 3 rpm
  monitorpb_DeviceMonitorTypeOptions_DEVICE_MONITOR_TYPE_FAN_3 = 3,
  // [Power] general power voltage
  monitorpb_DeviceMonitorTypeOptions_DEVICE_MONITOR_TYPE_POWER = 10,
  // [Power Redundant 1 Temperature] power redundant 1 temperature
  monitorpb_DeviceMonitorTypeOptions_DEVICE_MONITOR_TYPE_POWER_REDUNDANT_1_TEMPERATURE = 11,
  // [Power Redundant 1 Current] power redundant 1 current
  monitorpb_DeviceMonitorTypeOptions_DEVICE_MONITOR_TYPE_POWER_REDUNDANT_1_CURRENT = 12,
  // [Power Redundant 1 Voltage] power redundant 1 voltage
  monitorpb_DeviceMonitorTypeOptions_DEVICE_MONITOR_TYPE_POWER_REDUNDANT_1_VOLTAGE = 13,
  // [Power Redundant 1 Consumption] power redundant 1 consumption
  monitorpb_DeviceMonitorTypeOptions_DEVICE_MONITOR_TYPE_POWER_REDUNDANT_1_CONSUMPTION = 14,
  // [Power Redundant 2 Temperature] power redundant 2 temperature
  monitorpb_DeviceMonitorTypeOptions_DEVICE_MONITOR_TYPE_POWER_REDUNDANT_2_TEMPERATURE = 15,
  // [Power Redundant 2 Current] power redundant 2 current
  monitorpb_DeviceMonitorTypeOptions_DEVICE_MONITOR_TYPE_POWER_REDUNDANT_2_CURRENT = 16,
  // [Power Redundant 2 Voltage] power redundant 2 voltage
  monitorpb_DeviceMonitorTypeOptions_DEVICE_MONITOR_TYPE_POWER_REDUNDANT_2_VOLTAGE = 17,
  // [Power Redundant 2 Consumption] power redundant 2 consumption
  monitorpb_DeviceMonitorTypeOptions_DEVICE_MONITOR_TYPE_POWER_REDUNDANT_2_CONSUMPTION = 18,
};

enum monitorpb_SystemMonitorTypeOptions {
  // [CPU] CPU utilization.
  monitorpb_SystemMonitorTypeOptions_SYSTEM_MONITOR_TYPE_CPU_UTILIZATION = 0,
  // [Memory] Memory utilization.
  monitorpb_SystemMonitorTypeOptions_SYSTEM_MONITOR_TYPE_MEMORY_UTILIZATION = 1,
};

enum monitorpb_ValueTypeOptions {
  // [Int] the value is an int32.
  monitorpb_ValueTypeOptions_VALUE_TYPE_INT = 0,
  // [float] the value is a float32.
  monitorpb_ValueTypeOptions_VALUE_TYPE_FLOAT = 1,
  // [Int64] the value is an int64.
  monitorpb_ValueTypeOptions_VALUE_TYPE_INT64 = 2,
  // [float64] the value is an float64.
  monitorpb_ValueTypeOptions_VALUE_TYPE_FLOAT64 = 3,
};

enum monitorpb_LimitBoundaryTypeOptions {
  // [None] Has not supported the boundary settings.
  monitorpb_LimitBoundaryTypeOptions_LIMIT_BOUNDARY_TYPE_NONE = 0,
  // [Include] When the value includes the boundary will send an event.
  monitorpb_LimitBoundaryTypeOptions_LIMIT_BOUNDARY_TYPE_INCLUDE = 1,
  // [Exclude] When the value excludes the boundary will send an event.
  monitorpb_LimitBoundaryTypeOptions_LIMIT_BOUNDARY_TYPE_EXCLUDE = 2,
};

enum monitorpb_HardwareLEDTypeOptions {
  // [Power]
  monitorpb_HardwareLEDTypeOptions_HARDWARE_LED_TYPE_POWER = 0,
  // [System]
  monitorpb_HardwareLEDTypeOptions_HARDWARE_LED_TYPE_SYSTEM = 1,
  // [PoeMax]
  monitorpb_HardwareLEDTypeOptions_HARDWARE_LED_TYPE_POE_MAX = 2,
  // [Fan]
  monitorpb_HardwareLEDTypeOptions_HARDWARE_LED_TYPE_FAN = 3,
  // [Gps]
  monitorpb_HardwareLEDTypeOptions_HARDWARE_LED_TYPE_GPS = 4,
};

enum monitorpb_LEDColorTypeOptions {
  // [Green] The LED color is green.
  monitorpb_LEDColorTypeOptions_LED_COLOR_TYPE_GREEN = 0,
  // [Red] The LED and color is red.
  monitorpb_LEDColorTypeOptions_LED_COLOR_TYPE_RED = 1,
  // [Orange] The LED and color is orange.
  monitorpb_LEDColorTypeOptions_LED_COLOR_TYPE_ORANGE = 2,
};

enum monitorpb_LEDStateTypeOptions {
  // [Off] The LED is disabled.
  monitorpb_LEDStateTypeOptions_LED_STATE_TYPE_OFF = 0,
  // [On] The LED is enabled.
  monitorpb_LEDStateTypeOptions_LED_STATE_TYPE_ON = 1,
  // [Blinking] The LED is blinking.
  monitorpb_LEDStateTypeOptions_LED_STATE_TYPE_BLINKING = 2,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct monitorpb_Config {
  struct monitorpb_DeviceLimitConfig *DeviceLimitConfig;
  struct monitorpb_SystemLimitConfig *SystemLimitConfig;
};

struct monitorpb_DeviceLimitConfigEntry {
  // index for update, auto gen by `DeviceMonitorTypeOptions`
  enum monitorpb_DeviceMonitorTypeOptions MonitorOption;
  enum monitorpb_LimitBoundaryTypeOptions LimitOption;
  struct monitorpb_RangeValue *Boundary;
  enum monitorpb_ValueTypeOptions ValueOption;
  struct monitorpb_RangeValue *Value;
};

struct monitorpb_DeviceLimitConfig {
  unsigned int List_Len; // auto-gen: for list
  struct monitorpb_DeviceLimitConfigEntry **List;
};

struct monitorpb_SystemLimitConfigEntry {
  // index for update, auto gen by `SystemMonitorTypeOptions`
  enum monitorpb_SystemMonitorTypeOptions MonitorOption;
  enum monitorpb_LimitBoundaryTypeOptions LimitOption;
  struct monitorpb_RangeValue *Boundary;
  enum monitorpb_ValueTypeOptions ValueOption;
  struct monitorpb_RangeValue *Value;
};

struct monitorpb_SystemLimitConfig {
  unsigned int List_Len; // auto-gen: for list
  struct monitorpb_SystemLimitConfigEntry **List;
};

struct monitorpb_RangeValue {
  int32_t IntMin;
  int32_t IntMax;
  float FloatMin;
  float FloatMax;
  int64_t Int64Min;
  int64_t Int64Max;
  double Float64Min;
  double Float64Max;
};

struct monitorpb_DeviceScorllBarValueEntry {
  // index, to update the `DeviceLimitConfigEntry`
  enum monitorpb_DeviceMonitorTypeOptions MonitorOption;
  struct monitorpb_RangeValue *Value;
};

struct monitorpb_DeviceScorllBarValue {
  unsigned int List_Len; // auto-gen: for list
  struct monitorpb_DeviceScorllBarValueEntry **List;
};

struct monitorpb_SystemScorllBarValueEntry {
  // index, to update the `SystemLimitConfigEntry`
  enum monitorpb_SystemMonitorTypeOptions MonitorOption;
  struct monitorpb_RangeValue *Value;
};

struct monitorpb_SystemScorllBarValue {
  unsigned int List_Len; // auto-gen: for list
  struct monitorpb_SystemScorllBarValueEntry **List;
};

struct monitorpb_Status {
  struct monitorpb_DeviceStatus *DeviceStatus;
  struct monitorpb_SystemStatus *SystemStatus;
  struct monitorpb_LEDStatus *LedStatus;
};

struct monitorpb_DeviceStatusEntry {
  enum monitorpb_DeviceMonitorTypeOptions MonitorOption;
  struct monitorpb_DisplayValue *Value;
  enum eventpb_LoggingTypeOptions LoggingOption;
};

struct monitorpb_DeviceStatus {
  unsigned int List_Len; // auto-gen: for list
  struct monitorpb_DeviceStatusEntry **List;
};

struct monitorpb_SystemStatusEntry {
  enum monitorpb_SystemMonitorTypeOptions MonitorOption;
  struct monitorpb_DisplayValue *Value;
  enum eventpb_LoggingTypeOptions LoggingOption;
};

struct monitorpb_SystemStatus {
  unsigned int List_Len; // auto-gen: for list
  struct monitorpb_SystemStatusEntry **List;
};

struct monitorpb_DisplayValue {
  char *ValueUnit;
  int32_t IntValue;
  float FloatValue;
  int64_t Int64Value;
  double Float64Value;
};

struct monitorpb_LEDStatus {
  unsigned int SystemList_Len; // auto-gen: for list
  struct monitorpb_SystemLEDStatusEntry **SystemList;
  unsigned int PortList_Len; // auto-gen: for list
  struct monitorpb_PortLEDStatusEntry **PortList;
};

struct monitorpb_SystemLEDStatusEntry {
  enum monitorpb_HardwareLEDTypeOptions LedType;
  struct monitorpb_LEDStateEntry *LedState;
};

struct monitorpb_PortLEDStatusEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool IsSfp;
  struct monitorpb_LEDStateEntry *LinkLedState;
  bool IsSupportPoELed;
  struct monitorpb_LEDStateEntry *PoELedState;
  bool IsSupportSpeedLed;
  struct monitorpb_LEDStateEntry *SpeedLedState;
};

struct monitorpb_LEDStateEntry {
  enum monitorpb_LEDStateTypeOptions StateOption;
  enum monitorpb_LEDColorTypeOptions ColorOption;
  // for the `state_option` is `LED_STATE_TYPE_BLINKING`
  int32_t BlinkingIntervalMs;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_monitor_monitor
