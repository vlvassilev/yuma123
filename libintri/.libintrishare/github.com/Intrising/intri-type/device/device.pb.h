
#ifndef _H_intri_pb_github_com_Intrising_intri_type_device_device
#define _H_intri_pb_github_com_Intrising_intri_type_device_device

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../github.com/Intrising/intri-type/common/common.pb.h"
#include "../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
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

enum devicepb_RCLKTypeOptions {
  // [RCLK 0] RCLK 0
  devicepb_RCLKTypeOptions_RCLK_TYPE_0 = 0,
  // [RCLK 1] RCLK 1
  devicepb_RCLKTypeOptions_RCLK_TYPE_1 = 1,
};

enum devicepb_InputClockIndexTypeOptions {
  // [Clock Index 0] Clock Index 0
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_0 = 0,
  // [Clock Index 1] Clock Index 1
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_1 = 1,
  // [Clock Index 2] Clock Index 2
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_2 = 2,
  // [Clock Index 3] Clock Index 3
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_3 = 3,
  // [Clock Index 4] Clock Index 4
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_4 = 4,
  // [Clock Index 5] Clock Index 5
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_5 = 5,
  // [Clock Index 6] Clock Index 6
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_6 = 6,
  // [Clock Index 7] Clock Index 7
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_7 = 7,
  // [Clock Index 8] Clock Index 8
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_8 = 8,
  // [Clock Index 9] Clock Index 9
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_9 = 9,
  // [Clock Index 10] Clock Index 10
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_10 = 10,
  // [Clock Index 11] Clock Index 11
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_11 = 11,
  // [Clock Index 12] Clock Index 12
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_12 = 12,
  // [Clock Index 13] Clock Index 13
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_13 = 13,
  // [Clock Index 14] Clock Index 14
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_14 = 14,
  // [Clock Index 15] Clock Index 15
  devicepb_InputClockIndexTypeOptions_INPUT_CLOCK_INDEX_TYPE_15 = 15,
};

enum devicepb_InputClockTypeOptions {
  // [MAC Clock 0] MAC Clock 0
  devicepb_InputClockTypeOptions_INPUT_CLOCK_TYPE_MAC_CLOCK_0 = 0,
  // [MAC Clock 1] MAC Clock 1
  devicepb_InputClockTypeOptions_INPUT_CLOCK_TYPE_MAC_CLOCK_1 = 1,
  // [PHY A Clock 0] PHY A Clock 0
  devicepb_InputClockTypeOptions_INPUT_CLOCK_TYPE_PHY_A_CLOCK_0 = 2,
  // [PHY A Clock 1] PHY A Clock 1
  devicepb_InputClockTypeOptions_INPUT_CLOCK_TYPE_PHY_A_CLOCK_1 = 3,
  // [PHY B Clock 0] PHY B Clock 0
  devicepb_InputClockTypeOptions_INPUT_CLOCK_TYPE_PHY_B_CLOCK_0 = 4,
  // [PHY B Clock 1] PHY B Clock 1
  devicepb_InputClockTypeOptions_INPUT_CLOCK_TYPE_PHY_B_CLOCK_1 = 5,
  // [PHY C Clock 0] PHY C Clock 0
  devicepb_InputClockTypeOptions_INPUT_CLOCK_TYPE_PHY_C_CLOCK_0 = 6,
  // [PHY C Clock 1] PHY C Clock 1
  devicepb_InputClockTypeOptions_INPUT_CLOCK_TYPE_PHY_C_CLOCK_1 = 7,
  // [Bits In] Bits In
  devicepb_InputClockTypeOptions_INPUT_CLOCK_TYPE_BITS_IN = 8,
  // [1pps GPS] 1pps GPS
  devicepb_InputClockTypeOptions_INPUT_CLOCK_TYPE_1PPS_GPS = 10,
  // [1pps In] 1pps In
  devicepb_InputClockTypeOptions_INPUT_CLOCK_TYPE_1PPS_IN = 11,
  // [10Mhz In] 10Mhz In
  devicepb_InputClockTypeOptions_INPUT_CLOCK_TYPE_10MHZ_IN = 12,
  // [1pps Feedback] 1pps Feedback
  devicepb_InputClockTypeOptions_INPUT_CLOCK_TYPE_1PPS_FEEDBACK = 13,
};

enum devicepb_LedTypeOptions {
  // [Off]
  devicepb_LedTypeOptions_LED_TYPE_OFF = 0,
  // [On]
  devicepb_LedTypeOptions_LED_TYPE_ON = 1,
  // [Orange]
  devicepb_LedTypeOptions_LED_TYPE_ORANGE = 2,
  // [Green]
  devicepb_LedTypeOptions_LED_TYPE_GREEN = 3,
};

enum devicepb_PhyInterfaceTypeOptions {
  // [Smi]
  devicepb_PhyInterfaceTypeOptions_PHY_INTERFACE_TYPE_SMI = 0,
  // [Xsmi]
  devicepb_PhyInterfaceTypeOptions_PHY_INTERFACE_TYPE_XSMI = 1,
  // [Unused]
  devicepb_PhyInterfaceTypeOptions_PHY_INTERFACE_TYPE_UNUSED = 2,
};

enum devicepb_FactoryHwFeatureTypeOptions {
  // [PoE++] Power over Ethernet plus (30W ports) supported
  devicepb_FactoryHwFeatureTypeOptions_FACTORY_HW_FEATURE_TYPE_POE_PLUS_PLUS = 0,
  // [PoE+] Power over Ethernet plus (30W ports) supported
  devicepb_FactoryHwFeatureTypeOptions_FACTORY_HW_FEATURE_TYPE_POE_PLUS = 1,
  // [EEE] Energy Efficient Ethernet
  devicepb_FactoryHwFeatureTypeOptions_FACTORY_HW_FEATURE_TYPE_EEE = 2,
  // [RTC] Local real time clock
  devicepb_FactoryHwFeatureTypeOptions_FACTORY_HW_FEATURE_TYPE_RTC = 3,
  // [SFP] Pluggable optical port
  devicepb_FactoryHwFeatureTypeOptions_FACTORY_HW_FEATURE_TYPE_SFP = 4,
};

enum devicepb_PortPropertyTypeOptions {
  // [10M FULL] This port is capable of running at 10Mbit/s
  devicepb_PortPropertyTypeOptions_PORT_PROPERTIES_TYPE_10M_FULL = 0,
  // [10M HALF] This port is capable of running at 10Mbit/s
  devicepb_PortPropertyTypeOptions_PORT_PROPERTIES_TYPE_10M_HALF = 1,
  // [100M FULL] This port is capable of running at 100Mbit/s
  devicepb_PortPropertyTypeOptions_PORT_PROPERTIES_TYPE_100M_FULL = 2,
  // [100M HALF] This port is capable of running at 100Mbit/s
  devicepb_PortPropertyTypeOptions_PORT_PROPERTIES_TYPE_100M_HALF = 3,
  // [1G FULL] This port is capable of running at 1000Mbit/s
  devicepb_PortPropertyTypeOptions_PORT_PROPERTIES_TYPE_1000M_FULL = 4,
  // [2.5G FULL] This port is capable of running at 2500Mbit/s
  devicepb_PortPropertyTypeOptions_PORT_PROPERTIES_TYPE_2500M_FULL = 5,
  // [5G FULL] This port is capable of running at 5Gbit/s
  devicepb_PortPropertyTypeOptions_PORT_PROPERTIES_TYPE_5G_FULL = 6,
  // [10G FULL] This port is capable of running at 10Gbit/s
  devicepb_PortPropertyTypeOptions_PORT_PROPERTIES_TYPE_10G_FULL = 7,
  // [25G FULL] This port is capable of running at 25Gbit/s
  devicepb_PortPropertyTypeOptions_PORT_PROPERTIES_TYPE_25G_FULL = 8,
  // [RJ45] This port uses as RJ45 connector
  devicepb_PortPropertyTypeOptions_PORT_PROPERTIES_TYPE_RJ45 = 9,
  // [SFP] This port uses a pluggable SFP
  devicepb_PortPropertyTypeOptions_PORT_PROPERTIES_TYPE_SFP = 10,
  // [PoE] This port is capable to supply Power over Ethernet (PoE)
  devicepb_PortPropertyTypeOptions_PORT_PROPERTIES_TYPE_POE = 11,
  // [PoE+] This port is capable to supply Power over Ethernet Extended (PoE+)
  devicepb_PortPropertyTypeOptions_PORT_PROPERTIES_TYPE_POE_PLUS = 12,
  // [PoE++] This port is capable to supply Power over Ethernet Extended (PoE+)
  devicepb_PortPropertyTypeOptions_PORT_PROPERTIES_TYPE_POE_PLUS_PLUS = 13,
  // [Link Port] This port is capable to supply Power over Ethernet Extended (PoE+)
  devicepb_PortPropertyTypeOptions_PORT_PROPERTIES_TYPE_LINK_PORT = 14,
};

enum devicepb_DevicePortSpeedDuplexTypeOptions {
  // [Auto]
  devicepb_DevicePortSpeedDuplexTypeOptions_DEVICE_PORT_SPEED_DUPLEX_TYPE_AUTO = 0,
  // [10 Mbps / Full]
  devicepb_DevicePortSpeedDuplexTypeOptions_DEVICE_PORT_SPEED_DUPLEX_TYPE_10M_FULL = 1,
  // [10 Mbps / Half]
  devicepb_DevicePortSpeedDuplexTypeOptions_DEVICE_PORT_SPEED_DUPLEX_TYPE_10M_HALF = 2,
  // [100 Mbps / Full]
  devicepb_DevicePortSpeedDuplexTypeOptions_DEVICE_PORT_SPEED_DUPLEX_TYPE_100M_FULL = 3,
  // [100 Mbps / Half]
  devicepb_DevicePortSpeedDuplexTypeOptions_DEVICE_PORT_SPEED_DUPLEX_TYPE_100M_HALF = 4,
  // [1 Gbps  / Full]
  devicepb_DevicePortSpeedDuplexTypeOptions_DEVICE_PORT_SPEED_DUPLEX_TYPE_1000M_FULL = 5,
  // [2.5 Gbps / Full]
  devicepb_DevicePortSpeedDuplexTypeOptions_DEVICE_PORT_SPEED_DUPLEX_TYPE_2500M_FULL = 6,
  // [5 Gbps / Full]
  devicepb_DevicePortSpeedDuplexTypeOptions_DEVICE_PORT_SPEED_DUPLEX_TYPE_5G_FULL = 7,
  // [10 Gbps / Full]
  devicepb_DevicePortSpeedDuplexTypeOptions_DEVICE_PORT_SPEED_DUPLEX_TYPE_10G_FULL = 8,
  // [25 Gbps / Full]
  devicepb_DevicePortSpeedDuplexTypeOptions_DEVICE_PORT_SPEED_DUPLEX_TYPE_25G_FULL = 9,
  // [40 Gbps / Full]
  devicepb_DevicePortSpeedDuplexTypeOptions_DEVICE_PORT_SPEED_DUPLEX_TYPE_40G_FULL = 10,
  // [100 Gbps / Full]
  devicepb_DevicePortSpeedDuplexTypeOptions_DEVICE_PORT_SPEED_DUPLEX_TYPE_100G_FULL = 11,
  // [NA]
  devicepb_DevicePortSpeedDuplexTypeOptions_DEVICE_PORT_SPEED_DUPLEX_TYPE_NA = 12,
};

enum devicepb_PortInterfaceTypeOptions {
  // [Copper] Normal copper interface
  devicepb_PortInterfaceTypeOptions_PORT_INTERFACE_TYPE_COPPER = 0,
  // [Optical] Optical interface
  devicepb_PortInterfaceTypeOptions_PORT_INTERFACE_TYPE_OPTICAL = 1,
};

enum devicepb_DeviceTypeOptions {
  // [MCU] MCU
  devicepb_DeviceTypeOptions_DEVICE_TYPE_MCU = 0,
  // [I2C] I2C
  devicepb_DeviceTypeOptions_DEVICE_TYPE_I2C = 1,
  // [GPIO] GPIO
  devicepb_DeviceTypeOptions_DEVICE_TYPE_GPIO = 2,
};

enum devicepb_MonitorTypeOptions {
  // [Temperature Normal] Temperature Normal
  devicepb_MonitorTypeOptions_MONITOR_TYPE_TEMP_OK = 0,
  // [Temperature Warning] Temperature Warning
  devicepb_MonitorTypeOptions_MONITOR_TYPE_TEMP_WARN = 1,
  // [Fan Speed Ok] Fan Speed Ok
  devicepb_MonitorTypeOptions_MONITOR_TYPE_FAN_OK = 2,
  // [Fan Speed Warn] Fan Speed Warn
  devicepb_MonitorTypeOptions_MONITOR_TYPE_FAN_WARN = 3,
};

enum devicepb_PSUTypeOptions {
  // [Location Ok] Location Ok
  devicepb_PSUTypeOptions_PSU_TYPE_LOCATION_OK = 0,
  // [Location Warn] Location Warn
  devicepb_PSUTypeOptions_PSU_TYPE_LOCATION_WARN = 1,
  // [AC Ok] AC Ok
  devicepb_PSUTypeOptions_PSU_TYPE_AC_OK = 2,
  // [AC Warning] AC Warning
  devicepb_PSUTypeOptions_PSU_TYPE_AC_WARN = 3,
  // [Power Ok] Power Ok
  devicepb_PSUTypeOptions_PSU_TYPE_PW_OK = 4,
  // [Power Warning] Power Warning
  devicepb_PSUTypeOptions_PSU_TYPE_PW_WARN = 5,
  // [PSU Failed] Failed
  devicepb_PSUTypeOptions_PSU_TYPE_FAIL = 6,
};

enum devicepb_FanTypeOptions {
  // [RPM] RPM
  devicepb_FanTypeOptions_FAN_TYPE_RPM = 0,
};

enum devicepb_FanDeviceTypeOptions {
  // [RPM] RPM
  devicepb_FanDeviceTypeOptions_FAN_DEVICE_TYPE_RPM = 0,
};

enum devicepb_PoEDeviceTypeOptions {
  // [I2C] I2C
  devicepb_PoEDeviceTypeOptions_POE_DEVICE_TYPE_I2C = 0,
  // [UART] UART
  devicepb_PoEDeviceTypeOptions_POE_DEVICE_TYPE_UART = 1,
};

enum devicepb_BoundaryTypeOptions {
  // [Int]
  devicepb_BoundaryTypeOptions_BOUNDARY_TYPE_INT = 0,
  // [Double]
  devicepb_BoundaryTypeOptions_BOUNDARY_TYPE_DOUBLE = 1,
};

enum devicepb_LayerTypeOptions {
  // [L2]
  devicepb_LayerTypeOptions_LAYER_TYPE_L2 = 0,
  // [L3]
  devicepb_LayerTypeOptions_LAYER_TYPE_L3 = 1,
};

enum devicepb_InterfaceTypeOptions {
  // [VLAN]
  devicepb_InterfaceTypeOptions_INTERFACE_TYPE_VLAN = 0,
  // [Port]
  devicepb_InterfaceTypeOptions_INTERFACE_TYPE_PORT = 1,
  // [LAG]
  devicepb_InterfaceTypeOptions_INTERFACE_TYPE_TRUNK = 2,
  // [Multicast]
  devicepb_InterfaceTypeOptions_INTERFACE_TYPE_MULTICAST = 3,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

enum devicepb_LedInfo_Device_Union_Options {
  devicepb_LedInfo_Device_Union_Options_LEDDevice,
  devicepb_LedInfo_Device_Union_Options_I2CDevice,
  devicepb_LedInfo_Device_Union_Options_GPIODevice,
};
struct devicepb_LedInfo {
  char *Name;
  int32_t PortNo;
  enum devicepb_DeviceTypeOptions Type;
  char *Direction;
  enum devicepb_LedInfo_Device_Union_Options Device_Union_Option;
  union {
    struct devicepb_MCULedDevice *Device_LEDDevice;
    struct devicepb_I2CDevice *Device_I2CDevice;
    struct devicepb_GPIODevice *Device_GPIODevice;
  } Device;
};

enum devicepb_MonitorInfo_Device_Union_Options {
  devicepb_MonitorInfo_Device_Union_Options_MonitorDevice,
  devicepb_MonitorInfo_Device_Union_Options_I2CDevice,
  devicepb_MonitorInfo_Device_Union_Options_GPIODevice,
};
struct devicepb_MonitorInfo {
  char *Name;
  enum devicepb_DeviceTypeOptions Type;
  char *Direction;
  enum devicepb_MonitorInfo_Device_Union_Options Device_Union_Option;
  union {
    struct devicepb_MCUMonitorDevice *Device_MonitorDevice;
    struct devicepb_I2CDevice *Device_I2CDevice;
    struct devicepb_GPIODevice *Device_GPIODevice;
  } Device;
};

enum devicepb_PSUInfo_Device_Union_Options {
  devicepb_PSUInfo_Device_Union_Options_PSUDevice,
  devicepb_PSUInfo_Device_Union_Options_I2CDevice,
  devicepb_PSUInfo_Device_Union_Options_GPIODevice,
};
struct devicepb_PSUInfo {
  char *Name;
  enum devicepb_DeviceTypeOptions Type;
  char *Direction;
  enum devicepb_PSUInfo_Device_Union_Options Device_Union_Option;
  union {
    struct devicepb_MCUPSUDevice *Device_PSUDevice;
    struct devicepb_I2CDevice *Device_I2CDevice;
    struct devicepb_GPIODevice *Device_GPIODevice;
  } Device;
};

struct devicepb_MCULedDevice {
  int32_t I2CAddr;
  int32_t AddrRegister;
  enum devicepb_LedTypeOptions LedAction;
  int32_t ActiveBit;
};

struct devicepb_MCUMonitorDevice {
  int32_t I2CAddr;
  int32_t AddrRegister;
  enum devicepb_MonitorTypeOptions MonitorAction;
  int32_t ActiveBit;
};

struct devicepb_MCUPSUDevice {
  int32_t I2CAddr;
  int32_t AddrRegister;
  enum devicepb_PSUTypeOptions PSUAction;
  int32_t ActiveBit;
};

struct devicepb_MCUFanDevice {
  int32_t I2CAddr;
  int32_t AddrRegister;
  enum devicepb_FanTypeOptions FANAction;
  int32_t ActiveBit;
};

struct devicepb_I2CDevice {
  int32_t I2CAddr;
  int32_t AddrRegister;
  int32_t Action;
  int32_t ActiveBit;
};

struct devicepb_GPIODevice {
  int32_t Register;
  int32_t Pin;
  bool Action;
  bool Enable;
};

struct devicepb_I2Cinfo {
  char *Name;
  char *Direction;
  struct devicepb_I2CDevice *Device;
};

struct devicepb_GPIOInfo {
  char *Name;
  char *Direction;
  struct devicepb_GPIODevice *Device;
};

struct devicepb_PTPClockSynchronizerInfo {
  char *Name;
  char *Direction;
  struct devicepb_I2CDevice *Device;
};

struct devicepb_GPSInfo {
  char *Name;
  char *Path;
  char *Direction;
};

struct devicepb_ToDInfo {
  char *Name;
  char *Path;
  char *Direction;
};

struct devicepb_UARTDevice {
  char *Name;
  char *Path;
  char *Direction;
  int32_t Bandwidth;
};

enum devicepb_FanInfo_Device_Union_Options {
  devicepb_FanInfo_Device_Union_Options_FANDevice,
  devicepb_FanInfo_Device_Union_Options_I2CDevice,
  devicepb_FanInfo_Device_Union_Options_GPIODevice,
};
struct devicepb_FanInfo {
  char *Name;
  enum devicepb_FanDeviceTypeOptions Type;
  int32_t FanNumber;
  enum devicepb_FanInfo_Device_Union_Options Device_Union_Option;
  union {
    struct devicepb_MCUFanDevice *Device_FANDevice;
    struct devicepb_I2CDevice *Device_I2CDevice;
    struct devicepb_GPIODevice *Device_GPIODevice;
  } Device;
};

enum devicepb_PoEInfo_Device_Union_Options {
  devicepb_PoEInfo_Device_Union_Options_I2CDevice,
  devicepb_PoEInfo_Device_Union_Options_UartDevice,
};
struct devicepb_PoEInfo {
  char *Name;
  enum devicepb_PoEDeviceTypeOptions Type;
  enum devicepb_PoEInfo_Device_Union_Options Device_Union_Option;
  union {
    struct devicepb_I2CDevice *Device_I2CDevice;
    struct devicepb_UARTDevice *Device_UartDevice;
  } Device;
};

struct devicepb_HardwareInfo {
  unsigned int DeviceLed_Len; // auto-gen: for list
  struct devicepb_LedInfo **DeviceLed;
  unsigned int DeviceHWMonitor_Len; // auto-gen: for list
  struct devicepb_MonitorInfo **DeviceHWMonitor;
  unsigned int DevicePSU_Len; // auto-gen: for list
  struct devicepb_PSUInfo **DevicePSU;
  unsigned int DeviceI2C_Len; // auto-gen: for list
  struct devicepb_I2Cinfo **DeviceI2C;
  unsigned int DeviceHW_Len; // auto-gen: for list
  struct devicepb_GPIOInfo **DeviceHW;
  unsigned int DevicePTP_Len; // auto-gen: for list
  struct devicepb_PTPClockSynchronizerInfo **DevicePTP;
  unsigned int DeviceGPS_Len; // auto-gen: for list
  struct devicepb_GPSInfo **DeviceGPS;
  unsigned int DeviceToD_Len; // auto-gen: for list
  struct devicepb_ToDInfo **DeviceToD;
  unsigned int DeviceFan_Len; // auto-gen: for list
  struct devicepb_FanInfo **DeviceFan;
  unsigned int DevicePoE_Len; // auto-gen: for list
  struct devicepb_PoEInfo **DevicePoE;
};

struct devicepb_HardwareTableSize {
  int32_t TcamSize;
  int32_t BridgeFDBEntries;
  int32_t VirtualPorts;
  int32_t VirtualBridgeDomains;
  int32_t ARPTableEntries;
  int32_t RouterIPv4HostEntries;
  int32_t RouterIPv6HostEntries;
  int32_t RouterNextHopEntries;
  int32_t MulticastPhysicalPortGroups;
  int32_t MulticastLinkedListEntries;
  int32_t CentralizedCounters;
  int32_t SpanningTreeGroups;
  int32_t QoSProfiles;
  int32_t L2L3PortIsolation;
};

struct devicepb_PhyInterface {
  int32_t Interface;
  enum devicepb_PhyInterfaceTypeOptions Type;
};

struct devicepb_PortInfo {
  int32_t PortNo;
  int32_t MACChip;
  int32_t MACNo;
  int32_t PortGroup;
  int32_t PhyID1;
  int32_t PhyID2;
  struct devicepb_PhyInterface *PhyInterface;
  int32_t PoENo;
  unsigned int PoEChannel_Len; // auto-gen: for list
  int32_t *PoEChannel;
  int32_t PoEChipNo;
  int32_t MacsecEncLen;
  enum devicepb_PortInterfaceTypeOptions InterfaceType;
  unsigned int Properties_Len; // auto-gen: for list
  enum devicepb_PortPropertyTypeOptions *Properties;
  int32_t DeviceID;
  unsigned int SpeedProperties_Len; // auto-gen: for list
  enum devicepb_DevicePortSpeedDuplexTypeOptions *SpeedProperties;
};

// BoardInfo is the information that can not be modified
struct devicepb_BoardInfo {
  // https://oidref.com/1.3.6.1.2.1.1.1
  char *SystemDescription;
  unsigned int PortLists_Len; // auto-gen: for list
  struct devicepb_PortInfo **PortLists;
  unsigned int HwFeatures_Len; // auto-gen: for list
  enum devicepb_FactoryHwFeatureTypeOptions *HwFeatures;
  struct devicepb_HardwareTableSize *HwSize;
  int32_t CPUPort;
  struct devicepb_HardwareInfo *BoardDevice;
  struct devicepb_TimeControlInfo *TimeControl;
  // https://oidref.com/1.3.6.1.2.1.1.2
  char *EnterpriseOID;
};

struct devicepb_TimeControlSpeedFrequencyEntry {
  enum devicepb_PortPropertyTypeOptions Speed;
  // for frequency is 0
  int64_t Frequency;
  int64_t M;
  int64_t N;
  int64_t Div;
};

struct devicepb_TimeControlPortInfoEntry {
  int32_t PortNo;
  enum devicepb_RCLKTypeOptions RCLKPin;
  int64_t Frequency;
  enum devicepb_InputClockIndexTypeOptions InputClock;
};

struct devicepb_TimeControlInfo_InputClockMappingEntry {
  char *Key;
  enum devicepb_InputClockIndexTypeOptions Value;
};

struct devicepb_TimeControlInfo {
  // string type is InputClockTypeOptions.String()
  unsigned int InputClockMapping_Len; // auto-gen: for list
  struct devicepb_TimeControlInfo_InputClockMappingEntry **InputClockMapping;
  unsigned int PortInfoList_Len; // auto-gen: for list
  struct devicepb_TimeControlPortInfoEntry **PortInfoList;
  unsigned int SpeedFrequencyList_Len; // auto-gen: for list
  struct devicepb_TimeControlSpeedFrequencyEntry **SpeedFrequencyList;
};

struct devicepb_Boundary {
  enum devicepb_BoundaryTypeOptions Type;
  int32_t Max;
  int32_t Min;
  double Lower;
  double Upper;
};

struct devicepb_BoundaryAll {
  struct devicepb_BoundaryVLAN *VLAN;
  struct devicepb_BoundaryAccess *Access;
  struct devicepb_BoundaryDDM *DDM;
  struct devicepb_BoundaryACL *ACL;
  struct devicepb_BoundaryLog *Log;
  struct devicepb_BoundaryMirroring *Mirroring;
  struct devicepb_BoundaryTRUNK *Trunk;
  struct devicepb_BoundaryFDB *FDB;
  struct devicepb_BoundaryQoS *QoS;
  struct devicepb_BoundaryMulticast *Multicast;
  struct devicepb_BoundaryPoE *PoE;
  struct devicepb_BoundaryFiles *Files;
  struct devicepb_BoundaryTimeRange *TimeRange;
  struct devicepb_BoundaryDHCPServer *DHCPServer;
  struct devicepb_BoundaryMonitor *Monitor;
};

struct devicepb_BoundaryDHCPServer {
  // min: 0, max: 32
  struct devicepb_Boundary *MACBased;
};

struct devicepb_BoundaryVLAN {
  struct devicepb_Boundary *VlanID;
  struct devicepb_Boundary *VlanFilter;
  struct devicepb_Boundary *ProtocolBased;
  struct devicepb_Boundary *MACBased;
  struct devicepb_Boundary *SubnetBased;
  struct devicepb_Boundary *TPIDsRange;
  struct devicepb_Boundary *ProtocolClasses;
  struct devicepb_Boundary *SelectiveQinQ;
  struct devicepb_Boundary *Mapping;
};

struct devicepb_BoundaryDDM {
  struct devicepb_Boundary *DdmTemperature;
  struct devicepb_Boundary *DdmVoltage;
  struct devicepb_Boundary *DdmTxBias;
  struct devicepb_Boundary *DdmRxPower;
  struct devicepb_Boundary *DdmTxPower;
};

struct devicepb_BoundaryACL {
  struct devicepb_Boundary *ACLs;
  struct devicepb_Boundary *ACEs;
  struct devicepb_Boundary *Binding;
  struct devicepb_Boundary *Flow;
  struct devicepb_Boundary *FlowRules;
  struct devicepb_Boundary *Rules;
};

struct devicepb_BoundaryLog {
  struct devicepb_Boundary *LogRotateSize;
  struct devicepb_Boundary *LogRotateFileCount;
  struct devicepb_Boundary *LogTargetList;
};

struct devicepb_BoundaryAccess {
  struct devicepb_Boundary *Users;
  struct devicepb_Boundary *UsersAssociatedGroups;
  struct devicepb_Boundary *Groups;
  struct devicepb_Boundary *GroupsPattern;
  struct devicepb_Boundary *Restrictions;
  struct devicepb_Boundary *Servers;
};

struct devicepb_BoundaryMirroring {
  struct devicepb_Boundary *Session;
};

struct devicepb_BoundaryFDB {
  struct devicepb_Boundary *FDBSize;
  struct devicepb_Boundary *AgeTime;
  struct devicepb_Boundary *PortLearningLimit;
  struct devicepb_Boundary *PortSecurityLearningLimit;
  struct devicepb_Boundary *ForwardLimit;
  struct devicepb_Boundary *DropLimit;
};

struct devicepb_BoundaryTRUNK {
  struct devicepb_Boundary *IDRange;
  struct devicepb_Boundary *PriorityRange;
  struct devicepb_Boundary *MemberRange;
};

struct devicepb_BoundaryMulticast {
  // cpss
  struct devicepb_Boundary *VIDXRange;
  // intri-multicast
  struct devicepb_Boundary *StaticEntries;
  struct devicepb_Boundary *DynamicEntries;
};

struct devicepb_BoundaryQoS {
  struct devicepb_Boundary *QueueListRange;
  struct devicepb_Boundary *CoSRange;
  struct devicepb_Boundary *DSCPRange;
  struct devicepb_Boundary *WRRRange;
};

struct devicepb_BoundaryPoE {
  struct devicepb_Boundary *BudgetRange;
};

struct devicepb_BoundaryFiles {
  struct devicepb_Boundary *CertificatedRange;
};

struct devicepb_BoundaryTimeRange {
  struct devicepb_Boundary *EntryRange;
};

struct devicepb_BoundaryMonitor {
  struct devicepb_Boundary *Temperature;
  struct devicepb_Boundary *Power;
  struct devicepb_Boundary *Cpu;
  struct devicepb_Boundary *Memory;
  struct devicepb_Boundary *PowerRedundantConsumption;
  struct devicepb_Boundary *PowerRedundantTemperature;
  struct devicepb_Boundary *PowerRedundantVoltage;
  struct devicepb_Boundary *PowerRedundantCurrent;
};

struct devicepb_FunctionControlAll {
  struct devicepb_FunctionControlVLAN *VLAN;
  struct devicepb_FunctionControlACL *Acl;
  struct devicepb_FunctionControlMirroring *Mirroring;
  struct devicepb_FunctionControlFDB *FDB;
};

struct devicepb_FunctionControlVLAN {
  bool Voice;
  bool SelectiveQinq;
  bool MACBased;
  bool SubnetBased;
  bool ProtocolBased;
  bool Translation;
};

struct devicepb_FunctionControlACL {
  bool TimeRange;
  bool Binding;
  bool FlowMirroring;
};

struct devicepb_FunctionControlMirroring {
  bool RSPAN;
};

struct devicepb_FunctionControlFDB {
  bool PortSecurity;
  bool LearningLimit;
  bool Forward;
  bool Drop;
};

struct devicepb_PathAll {
  char *LogRamDisk;
  char *LogFlash;
  char *ConfigDefault;
  char *ConfigSaved;
  char *User;
  char *IPTables;
  char *CoreReboot;
  char *WarmStart;
  char *VlanMgmt;
  char *TimeCfg;
  char *BootReady;
  char *IPv6LinkLocal;
  char *TimeZone;
  char *Resolv;
  char *DropbearProc;
  char *TelnetProc;
  char *FTPProc;
  char *LocalUser;
  char *Certificates;
  char *OpenSSLCertificates;
  char *SNMPdCertificates;
  char *SNMPdCertificatesKey;
  char *NetCfg;
  char *IPv4Script;
  char *IPv6Script;
  char *LastSaveTime;
  char *Image;
  char *AltVersion;
  char *AltBuildDateTime;
  char *Version;
  char *BuildDateTime;
  char *DhcpServerDnsmasqLeasePath;
  char *DhcpServerDnsmasqCfgPath;
  char *DhcpServerPortBasedInfo;
};

struct devicepb_InterfaceIdentify {
  enum devicepb_InterfaceTypeOptions Type;
  // DeviceID is pre-defined for switch stacking https://en.wikipedia.org/wiki/Stackable_switch
// pre-generated, not visible for User, the value will all be 0 for now
  int32_t DeviceID;
  int32_t PortNo;
  int32_t LAGNo;
  int32_t VlanID;
};

// for user interface like WEB, CLI, SNMP, we only need to show attributes below.
// - "model"
// - "m_a_c_addr"
// - "serial_no"
// - "hw_version"
struct devicepb_Info {
  char *Model;
  char *MACAddr;
  char *MACAddrFactory;
  char *SerialNo;
  char *Vendor;
  char *AltBuildDateTime;
  char *CurrentBuildDateTime;
  char *AltSwVersion;
  char *CurrentSwVersion;
  char *HwVersion;
  char *CurrentImage;
  char *Board;
  enum devicepb_LayerTypeOptions Layer;
};

struct devicepb_PortList {
  unsigned int List_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_device_device
