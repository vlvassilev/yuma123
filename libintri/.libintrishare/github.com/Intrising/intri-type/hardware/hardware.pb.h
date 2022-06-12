
#ifndef _H_intri_pb_github_com_Intrising_intri_type_hardware_hardware
#define _H_intri_pb_github_com_Intrising_intri_type_hardware_hardware

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../github.com/Intrising/intri-type/common/common.pb.h"
#include "../../../../github.com/Intrising/intri-type/device/device.pb.h"
#include "../../../../github.com/Intrising/intri-type/event/event.pb.h"
#include "../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include "../../../../github.com/golang/protobuf/ptypes/timestamp/timestamp.pb.h"
#include <stdbool.h>
#include <stdint.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum hardwarepb_PowerTypeOptions {
  // [REDUNDANT] Redundant Power
  hardwarepb_PowerTypeOptions_POWER_TYPE_REDUNDANT = 0,
  // [GENERAL] General Power
  hardwarepb_PowerTypeOptions_POWER_TYPE_GENERAL = 1,
};

enum hardwarepb_LEDModeTypeOptions {
  // [Dynamic] Display static states and blink when data is present on a port.
  hardwarepb_LEDModeTypeOptions_LED_MODE_TYPE_DYNAMIC = 0,
  // [Static] Display static states but do not blink with data.
  hardwarepb_LEDModeTypeOptions_LED_MODE_TYPE_STATIC = 1,
  // [Quiet Display] Desplay is reduced to sys and on LED,and Port LEDs remain off.
  hardwarepb_LEDModeTypeOptions_LED_MODE_TYPE_QUIET_DISPLAY = 2,
  // [All Dark] This mode is not recommended, as the unit may mistakenly be deemed powered down.
  hardwarepb_LEDModeTypeOptions_LED_MODE_TYPE_ALL_DARK = 3,
  // [Lightshow] Similar to a led_test but permanent. This may be turned on to easier locate a physical unit among others.
  hardwarepb_LEDModeTypeOptions_LED_MODE_TYPE_LIGHTSHOW = 4,
};

// *internal usage
enum hardwarepb_LEDBehaviorModeTypeOptions {
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_LINK1G = 0,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_LINK5G10G = 1,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_BLOCKED = 2,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_REJECT = 3,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_IDLE = 4,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_POE = 5,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_POEPLUS = 6,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_POEPLUSPLUS = 7,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_OFF = 8,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_SYS_NORMAL = 9,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_SYS_MINOR_ALARM = 10,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_SYS_MAJOR_ALARM = 11,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_USER_DEF1 = 12,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_USER_DEF2 = 13,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_COLOR_GREEN = 14,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_COLOR_ORANGE = 15,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_COLOR_RED = 16,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_COLOR_BLUE = 17,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_COLOR_MAGENTA = 18,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_REQUEST_NMP_IP = 19,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_FACTORY_DEFAULT_KEEP_NETWORK = 20,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_FACTORY_DEFAULT_KEEP_NONE = 21,
  // * internal
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_FACTORY_DEFAULT_NO_OPERATION = 22,
  // * internal blink 1 time a 200 milliseconds.
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_BLINKING_NORMAL = 23,
  // * internal blink 1 time a 500 milliseconds.
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_BLINKING_SLOW = 24,
  // * internal blink 1 time a 100 milliseconds.
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_BLINKING_FAST = 25,
  // * internal This LED is not support
  hardwarepb_LEDBehaviorModeTypeOptions_LED_BEHAVIOR_MODE_TYPE_NA = 9999,
};

// *internal usage
enum hardwarepb_LEDTypeOptions {
  hardwarepb_LEDTypeOptions_LED_TYPE_SYSTEM = 0,
  hardwarepb_LEDTypeOptions_LED_TYPE_GPS = 1,
  hardwarepb_LEDTypeOptions_LED_TYPE_POEMAX = 2,
  hardwarepb_LEDTypeOptions_LED_TYPE_FAN = 3,
  hardwarepb_LEDTypeOptions_LED_TYPE_POE = 4,
  hardwarepb_LEDTypeOptions_LED_TYPE_SFP = 5,
};

enum hardwarepb_PoEPriorityLevelTypeOptions {
  // [Unknown] Priority unknow
  hardwarepb_PoEPriorityLevelTypeOptions_POE_PRIORITY_LEVEL_TYPE_UNKNOWN = 0,
  // [Low] Priority low
  hardwarepb_PoEPriorityLevelTypeOptions_POE_PRIORITY_LEVEL_TYPE_LOW = 1,
  // [Medium] Priority medium
  hardwarepb_PoEPriorityLevelTypeOptions_POE_PRIORITY_LEVEL_TYPE_MEDIUM = 2,
  // [High] Priority high
  hardwarepb_PoEPriorityLevelTypeOptions_POE_PRIORITY_LEVEL_TYPE_HIGH = 3,
};

enum hardwarepb_PoEPortModeTypeOptions {
  // [PoE] 802.3af PoE
  hardwarepb_PoEPortModeTypeOptions_POE_PORT_MODE_TYPE_POE = 0,
  // [PoE+] 802.3at PoE+
  hardwarepb_PoEPortModeTypeOptions_POE_PORT_MODE_TYPE_POE_PLUS = 1,
  // [PoE++] 802.3bt PoE++
  hardwarepb_PoEPortModeTypeOptions_POE_PORT_MODE_TYPE_POE_PLUS_PLUS = 2,
  // *[PoE LLDP Control] PoE mode is controlled by LLDP
  hardwarepb_PoEPortModeTypeOptions_POE_PORT_MODE_TYPE_POE_LLDP = 3,
};

enum hardwarepb_PoEDeterminedClassTypeOptions {
  // [Class0] Class level 0
  hardwarepb_PoEDeterminedClassTypeOptions_POE_DETERMINED_CLASS_TYPE_CLASS0 = 0,
  // [Class1] Class level 1
  hardwarepb_PoEDeterminedClassTypeOptions_POE_DETERMINED_CLASS_TYPE_CLASS1 = 1,
  // [Class2] Class level 2
  hardwarepb_PoEDeterminedClassTypeOptions_POE_DETERMINED_CLASS_TYPE_CLASS2 = 2,
  // [Class3] Class level 3
  hardwarepb_PoEDeterminedClassTypeOptions_POE_DETERMINED_CLASS_TYPE_CLASS3 = 3,
  // [Class4] Class level 4
  hardwarepb_PoEDeterminedClassTypeOptions_POE_DETERMINED_CLASS_TYPE_CLASS4 = 4,
  // [Class5] Class level 5
  hardwarepb_PoEDeterminedClassTypeOptions_POE_DETERMINED_CLASS_TYPE_CLASS5 = 5,
  // [Class6] Class level 6
  hardwarepb_PoEDeterminedClassTypeOptions_POE_DETERMINED_CLASS_TYPE_CLASS6 = 6,
  // [Class7] Class level 7
  hardwarepb_PoEDeterminedClassTypeOptions_POE_DETERMINED_CLASS_TYPE_CLASS7 = 7,
  // [Class8] Class level 8
  hardwarepb_PoEDeterminedClassTypeOptions_POE_DETERMINED_CLASS_TYPE_CLASS8 = 8,
  // [Overload] Overload
  hardwarepb_PoEDeterminedClassTypeOptions_POE_DETERMINED_CLASS_TYPE_OVERLOAD = 9,
  // [Probes Not Equal] Probes not equal
  hardwarepb_PoEDeterminedClassTypeOptions_POE_DETERMINED_CLASS_TYPE_PROBES_NOT_EQUAL = 10,
  // [Unknown] Unknown
  hardwarepb_PoEDeterminedClassTypeOptions_POE_DETERMINED_CLASS_TYPE_UNKNOWN = 11,
};

enum hardwarepb_PoEConditionTypeOptions {
  // [Disabled] Disabled
  hardwarepb_PoEConditionTypeOptions_POE_CONDITION_TYPE_DISABLED = 0,
  // [Power Off] Power off
  hardwarepb_PoEConditionTypeOptions_POE_CONDITION_TYPE_POWER_OFF = 1,
  // [Dtypecovering] Dtypecovering
  hardwarepb_PoEConditionTypeOptions_POE_CONDITION_TYPE_DTYPECOVERING = 2,
  // [Powered] Powered
  hardwarepb_PoEConditionTypeOptions_POE_CONDITION_TYPE_POWERED = 3,
  // [Class Mtypematch] Class mtypematch
  hardwarepb_PoEConditionTypeOptions_POE_CONDITION_TYPE_CLASS_MTYPEMATCH = 4,
  // [Short Circuit] Short circuit
  hardwarepb_PoEConditionTypeOptions_POE_CONDITION_TYPE_SHORT_CIRCUIT = 5,
  // [Rejected] Rejected
  hardwarepb_PoEConditionTypeOptions_POE_CONDITION_TYPE_REJECTED = 6,
  // [Overload] Overload
  hardwarepb_PoEConditionTypeOptions_POE_CONDITION_TYPE_OVERLOAD = 7,
  // [Underload] Underload
  hardwarepb_PoEConditionTypeOptions_POE_CONDITION_TYPE_UNDERLOAD = 8,
  // [Over Temperature] Over temperature
  hardwarepb_PoEConditionTypeOptions_POE_CONDITION_TYPE_OVER_TEMP = 9,
  // [Voltage Too Low] Voltage too low
  hardwarepb_PoEConditionTypeOptions_POE_CONDITION_TYPE_VOLTAGE_TOO_LOW = 10,
  // [Voltage Too High] Voltage too high
  hardwarepb_PoEConditionTypeOptions_POE_CONDITION_TYPE_VOLTAGE_TOO_HIGH = 11,
  // [Unknow Error] Unknow error
  hardwarepb_PoEConditionTypeOptions_POE_CONDITION_TYPE_UNKNOW_ERROR = 12,
  // [Budget Exceeded] Budget exceeded
  hardwarepb_PoEConditionTypeOptions_POE_CONDITION_TYPE_BUDGET_EXCEEDED = 13,
  // [None Standard PD Type] None standard PD type
  hardwarepb_PoEConditionTypeOptions_POE_CONDITION_TYPE_TYPE_NON_STANDARD_PD = 14,
};

enum hardwarepb_GPSStatusTypeOptions {
  // [Disabled] Disabled
  hardwarepb_GPSStatusTypeOptions_GPS_STATUS_TYPE_DISABLED = 0,
  // [Tracking] Tracking
  hardwarepb_GPSStatusTypeOptions_GPS_STATUS_TYPE_TRACKING = 1,
  // [Sync] Sync
  hardwarepb_GPSStatusTypeOptions_GPS_STATUS_TYPE_SYNC = 2,
};

enum hardwarepb_DLDPAuthCationModeTypeOptions {
  hardwarepb_DLDPAuthCationModeTypeOptions_DLDP_AUTHCATION_MODE_TYPE_NONE = 0,
  hardwarepb_DLDPAuthCationModeTypeOptions_DLDP_AUTHCATION_MODE_TYPE_PLAINTEXT = 1,
  hardwarepb_DLDPAuthCationModeTypeOptions_DLDP_AUTHCATION_MODE_TYPE_MD5 = 2,
};

enum hardwarepb_ShutdownModeTypeOptions {
  hardwarepb_ShutdownModeTypeOptions_SHUT_MODE_TYPE_AUTO = 0,
  hardwarepb_ShutdownModeTypeOptions_SHUT_MODE_TYPE_MANUAL = 1,
};

enum hardwarepb_DLDPWorkModeTypeOptions {
  hardwarepb_DLDPWorkModeTypeOptions_DLDP_WORK_MODE_TYPE_NORMAL = 0,
  hardwarepb_DLDPWorkModeTypeOptions_DLDP_WORK_MODE_TYPE_ENHANCE = 1,
};

enum hardwarepb_DLDPNeighobrStateTypeOptions {
  hardwarepb_DLDPNeighobrStateTypeOptions_DLDP_NEIGHBOR_STATE_TYPE_UNCONFIRM = 0,
  hardwarepb_DLDPNeighobrStateTypeOptions_DLDP_NEIGHBOR_STATE_TYPE_CONFIRM = 1,
};

enum hardwarepb_DLDPProtocolStateTypeOptions {
  hardwarepb_DLDPProtocolStateTypeOptions_DLDP_PROTOCOL_STATE_TYPE_INITAL = 0,
  hardwarepb_DLDPProtocolStateTypeOptions_DLDP_PROTOCOL_STATE_TYPE_INACTIVE = 1,
  hardwarepb_DLDPProtocolStateTypeOptions_DLDP_PROTOCOL_STATE_TYPE_ACTIVE = 2,
  hardwarepb_DLDPProtocolStateTypeOptions_DLDP_PROTOCOL_STATE_TYPE_ADVERTISEMENT = 3,
  hardwarepb_DLDPProtocolStateTypeOptions_DLDP_PROTOCOL_STATE_TYPE_PROBE = 4,
  hardwarepb_DLDPProtocolStateTypeOptions_DLDP_PROTOCOL_STATE_TYPE_DISABLE = 5,
  hardwarepb_DLDPProtocolStateTypeOptions_DLDP_PROTOCOL_STATE_TYPE_DELAY_DOWN = 6,
};

enum hardwarepb_PowerRedundantTypeOptions {
  // [PSU A]
  hardwarepb_PowerRedundantTypeOptions_POWER_REDUNDANT_TYPE_A = 0,
  // [PSU B]
  hardwarepb_PowerRedundantTypeOptions_POWER_REDUNDANT_TYPE_B = 1,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct hardwarepb_IPv4Static {
  char *IPAddress;
  char *SubnetMask;
};

struct hardwarepb_DLDPConfig {
  struct hardwarepb_DLDPSystemConfig *System;
  struct hardwarepb_DLDPPortConfig *Port;
};

struct hardwarepb_DLDPSystemConfig {
  bool Enabled;
  int32_t AdvertisementInterval;
  enum hardwarepb_ShutdownModeTypeOptions ShutDownMode;
  enum hardwarepb_DLDPAuthCationModeTypeOptions AuthMode;
  enum hardwarepb_DLDPWorkModeTypeOptions WorkMode;
};

struct hardwarepb_DLDPPortConfig {
  unsigned int List_Len; // auto-gen: for list
  struct hardwarepb_DLDPPortConfigEntry **List;
};

struct hardwarepb_DLDPPortConfigEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool Enabled;
};

struct hardwarepb_DLDPPortStatus {
  struct hardwarepb_DLDPPortStatusEntry *List;
};

struct hardwarepb_DLDPPortStatusEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool Enabled;
  enum hardwarepb_DLDPProtocolStateTypeOptions State;
  bool Link;
};

struct hardwarepb_DLDPNeighborEntry {
  char *NeighborMACAddress;
  int32_t PortNo;
  int32_t NeighborAgetime;
  enum hardwarepb_DLDPNeighobrStateTypeOptions NeighborState;
};

struct hardwarepb_EnableRequest {
  unsigned int List_Len; // auto-gen: for list
  struct hardwarepb_EnableRequestEntry **List;
};

struct hardwarepb_EnableRequestEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool Enabled;
};

struct hardwarepb_PortLED {
  unsigned int List_Len; // auto-gen: for list
  struct hardwarepb_PortLEDEntry **List;
};

struct hardwarepb_PortLEDEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  enum hardwarepb_LEDBehaviorModeTypeOptions State;
};

struct hardwarepb_BoardLED {
  enum hardwarepb_LEDBehaviorModeTypeOptions Power;
  enum hardwarepb_LEDBehaviorModeTypeOptions System;
  enum hardwarepb_LEDBehaviorModeTypeOptions PoEMax;
  enum hardwarepb_LEDBehaviorModeTypeOptions Fan;
  enum hardwarepb_LEDBehaviorModeTypeOptions GPS;
};

struct hardwarepb_LEDStatus {
  struct hardwarepb_PortLED *PortLED;
  struct hardwarepb_BoardLED *SystemLED;
};

struct hardwarepb_Coordinates {
  char *Latitude;
  char *Longitude;
};

struct hardwarepb_GNSSVerboseInfomation {
  char *Mode;
  char *NavigationalStatus;
  float Speed;
  float Course;
  float MagneticVariation;
};

struct hardwarepb_GPSEnable {
  bool Enabled;
};

struct hardwarepb_GPSStatus {
  bool Enabled;
  enum hardwarepb_GPSStatusTypeOptions Status;
  struct hardwarepb_GNSSVerboseInfomation *VerboseInfo;
  struct hardwarepb_Coordinates *Location;
  struct timestamppb_Timestamp *DateTime;
};

struct hardwarepb_GPSCommand {
  char *Request;
  unsigned int Respond_Len; // auto-gen: for list
  char **Respond;
};

struct hardwarepb_USBStatus {
  bool Enabled;
  bool Connected;
  char *ConnectedDevice;
};

struct hardwarepb_USBEthernetConfig {
  struct hardwarepb_IPv4Static *Static;
};

struct hardwarepb_FanSpeedConfig {
  unsigned int List_Len; // auto-gen: for list
  struct hardwarepb_FanSpeedConfigEntry **List;
};

struct hardwarepb_FanSpeedConfigEntry {
  int32_t FanNo;
  int32_t RotatingSpeed;
};

struct hardwarepb_FanStatus {
  unsigned int List_Len; // auto-gen: for list
  struct hardwarepb_FanStatusEntry **List;
};

struct hardwarepb_FanStatusEntry {
  int32_t FanNo;
  int32_t RotatingSpeedRPM;
};

struct hardwarepb_TemperatureStatus {
  unsigned int List_Len; // auto-gen: for list
  struct hardwarepb_TemperatureStatusEntry **List;
};

struct hardwarepb_TemperatureStatusEntry {
  char *Name;
  int32_t ID;
  int32_t Temperature;
};

struct hardwarepb_PowerRedundantState {
  // bool power_redundant_a_plug_in = 1;
// bool power_redundant_b_plug_in = 2;
// bool power_redundant_a_a_c = 3;
// bool power_redundant_b_a_c = 4;
// bool power_redundant_a = 5;
// bool power_redundant_b = 6;
// float power_redundant_a_voltage = 7;
// float power_redundant_a_current = 8;
// float power_redundant_a_power_consumption = 9;
// float power_redundant_a_temperature = 10;
// float power_redundant_b_voltage = 11;
// float power_redundant_b_current = 12;
// float power_redundant_b_power_consumption = 13;
// float power_redundant_b_temperature = 14;
  unsigned int RedundantList_Len; // auto-gen: for list
  struct hardwarepb_PowerRedundantStateEntry **RedundantList;
};

struct hardwarepb_PowerRedundantStateEntry {
  // [Power Redundant Type]
  enum hardwarepb_PowerRedundantTypeOptions PwOption;
  // plug in
  bool PowerRedundantPlugIn;
  // AC OK
  bool PowerRedundantACOk;
  // Power OK
  bool PowerRedundantOk;
  // Power Voltage
  float PowerRedundantVoltage;
  // Power Current
  float PowerRedundantCurrent;
  // Power Consumption
  float PowerRedundantPowerConsumption;
  // PSU Temperature
  float PowerRedundantTemperature;
};

struct hardwarepb_PowerGeneralState {
  bool Power_12V;
  bool Power_54V;
};

enum hardwarepb_PowerState_PowerType_Union_Options {
  hardwarepb_PowerState_PowerType_Union_Options_PowerRedundantState,
  hardwarepb_PowerState_PowerType_Union_Options_PowerGeneralState,
};
struct hardwarepb_PowerState {
  enum hardwarepb_PowerTypeOptions Type;
  enum hardwarepb_PowerState_PowerType_Union_Options PowerType_Union_Option;
  union {
    struct hardwarepb_PowerRedundantState *PowerType_PowerRedundantState;
    struct hardwarepb_PowerGeneralState *PowerType_PowerGeneralState;
  } PowerType;
};

struct hardwarepb_TimeControlRegister {
  int32_t RegBase;
  int32_t Offset;
  unsigned char *Data;
};

struct hardwarepb_Config {
  struct hardwarepb_LEDConfig *LED;
  struct hardwarepb_FanSpeedConfig *Fan;
  struct hardwarepb_GPSEnable *GPS;
};

struct hardwarepb_Ports {
  unsigned int List_Len; // auto-gen: for list
  struct hardwarepb_PortEntry **List;
};

struct hardwarepb_PortEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  int32_t HardwarePortNo;
  int32_t SwitchPortNo;
  enum devicepb_PortInterfaceTypeOptions InterfaceType;
  unsigned int Properies_Len; // auto-gen: for list
  enum devicepb_PortPropertyTypeOptions *Properies;
  bool SupportPoE;
  bool SupportSFP;
  enum hardwarepb_LEDBehaviorModeTypeOptions LinkLED;
  enum hardwarepb_LEDBehaviorModeTypeOptions PoELED;
  enum hardwarepb_LEDBehaviorModeTypeOptions SpeedLED;
};

struct hardwarepb_SystemInfo {
  struct hardwarepb_FanPercent *Fan;
  struct hardwarepb_TemperatureStatus *Temp;
  struct hardwarepb_BoardLED *BoardLED;
  struct hardwarepb_GPSStatus *GPS;
};

struct hardwarepb_LEDConfig {
  enum hardwarepb_LEDModeTypeOptions Mode;
  bool ResetButton;
};

struct hardwarepb_FanPercent {
  unsigned int List_Len; // auto-gen: for list
  struct hardwarepb_FanPercentEntry **List;
};

struct hardwarepb_FanPercentEntry {
  int32_t FanNo;
  int32_t PercentRPM;
};

struct hardwarepb_LEDType {
  enum hardwarepb_LEDBehaviorModeTypeOptions LED;
};

struct hardwarepb_SignalState {
  unsigned int List_Len; // auto-gen: for list
  struct hardwarepb_SignalStateEntry **List;
};

struct hardwarepb_SignalStateEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool State;
};

struct hardwarepb_SFPInfo {
  unsigned int List_Len; // auto-gen: for list
  struct hardwarepb_SFPInfoEntry **List;
};

struct hardwarepb_SFPInfoEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  struct eventpb_SFPInfo *Info;
};

struct hardwarepb_DeviceI2CAddress {
  unsigned int Bus0_Len; // auto-gen: for list
  char **Bus0;
  char *Mux;
  unsigned int List_Len; // auto-gen: for list
  struct hardwarepb_DeviceI2CAddressInfo **List;
};

struct hardwarepb_DeviceI2CAddressInfo {
  int32_t Ch;
  unsigned int Address_Len; // auto-gen: for list
  char **Address;
};

struct hardwarepb_InfoList {
  unsigned int Info_Len; // auto-gen: for list
  char **Info;
};

struct hardwarepb_Enable {
  bool Enabled;
};

struct hardwarepb_Network {
  char *IP;
  char *Mask;
};

struct hardwarepb_MCUInfo {
  unsigned int List_Len; // auto-gen: for list
  struct hardwarepb_MCUVersion **List;
};

struct hardwarepb_MCUVersion {
  char *Name;
  char *Version;
};

struct hardwarepb_PoESetting {
  unsigned int List_Len; // auto-gen: for list
  struct hardwarepb_PoESettingEntry **List;
};

struct hardwarepb_PoESettingEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool Enable;
  enum hardwarepb_PoEPortModeTypeOptions Mode;
  enum hardwarepb_PoEPriorityLevelTypeOptions Priority;
};

struct hardwarepb_PoEPortStatusList {
  unsigned int List_Len; // auto-gen: for list
  struct hardwarepb_PoEPortStatusEntry **List;
};

struct hardwarepb_PoEPortStatusEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool IsEnabled;
  bool IsLinkUp;
  enum hardwarepb_PoEDeterminedClassTypeOptions Class;
  double PowerConsumption;
  double Current;
  double Voltage;
  double Temperature;
  enum hardwarepb_PoEConditionTypeOptions Condition;
};

struct hardwarepb_PoESystemStatus {
  double MinShutDownVoltage;
  double MaxShutDownVoltage;
  double VMainVoltage;
  double IMainCurrent;
  double PowerConsumption;
  int32_t Budget;
  char *Version;
};

struct hardwarepb_PoESystemBudgetEntry {
  int32_t Budget;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_hardware_hardware
