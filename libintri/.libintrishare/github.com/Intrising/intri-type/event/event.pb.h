
#ifndef _H_intri_pb_github_com_Intrising_intri_type_event_event
#define _H_intri_pb_github_com_Intrising_intri_type_event_event

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum eventpb_ACLActionTypeOptions {
  // [Add] Add
  eventpb_ACLActionTypeOptions_ACL_ACTION_TYPE_ADD = 0,
  // [Delete] Delete
  eventpb_ACLActionTypeOptions_ACL_ACTION_TYPE_DELETE = 1,
};

// for aggregation changed
enum eventpb_AggrTypeOptions {
  // [Clear]
  eventpb_AggrTypeOptions_AGGR_TYPE_CLEAR = 0,
  // [Update]
  eventpb_AggrTypeOptions_AGGR_TYPE_UPDATE = 1,
};

enum eventpb_AggrSourceTypeOptions {
  // [Trunk]
  eventpb_AggrSourceTypeOptions_AGGR_SOURCE_TYPE_TRUNK = 0,
  // [Lgport]
  eventpb_AggrSourceTypeOptions_AGGR_SOURCE_TYPE_LGPORT = 1,
};

enum eventpb_AUParameterTypeOptions {
  // [Normal] Normal
  eventpb_AUParameterTypeOptions_AU_PARAMETER_TYPE_NORMAL = 0,
  // [Port Security] Port Security
  eventpb_AUParameterTypeOptions_AU_PARAMETER_TYPE_PORT_SECURITY = 2,
  // [Pacc] Pacc
  eventpb_AUParameterTypeOptions_AU_PARAMETER_TYPE_PACC = 3,
};

enum eventpb_FDBEntryActionTypeOptions {
  // [Add] Add
  eventpb_FDBEntryActionTypeOptions_FDB_ENTRY_ACTION_TYPE_ADD = 0,
  // [Delete] Delete
  eventpb_FDBEntryActionTypeOptions_FDB_ENTRY_ACTION_TYPE_DELETE = 1,
  // [Move] Move
  eventpb_FDBEntryActionTypeOptions_FDB_ENTRY_ACTION_TYPE_MOVE = 2,
};

enum eventpb_MulticastEntryTypeOptions {
  // [Mac] Mac
  eventpb_MulticastEntryTypeOptions_MULTICAST_ENTRY_TYPE_MAC = 0,
  // [Ipv4] Ipv4
  eventpb_MulticastEntryTypeOptions_MULTICAST_ENTRY_TYPE_IPV4 = 1,
  // [Ipv6] Ipv6
  eventpb_MulticastEntryTypeOptions_MULTICAST_ENTRY_TYPE_IPV6 = 2,
};

enum eventpb_ButtonTypeOptions {
  // [Reset]
  eventpb_ButtonTypeOptions_BUTTON_TYPE_RESET = 0,
};

enum eventpb_ButtonActionTypeOptions {
  // [Pressed]
  eventpb_ButtonActionTypeOptions_BUTTON_ACTION_TYPE_PRESSED = 0,
  // [Released]
  eventpb_ButtonActionTypeOptions_BUTTON_ACTION_TYPE_RELEASED = 1,
};

enum eventpb_ButtonTriggerActionTypeOptions {
  // [Reboot]
  eventpb_ButtonTriggerActionTypeOptions_BUTTON_TRIGGER_ACTION_TYPE_REBOOT = 0,
  // [Factory]
  eventpb_ButtonTriggerActionTypeOptions_BUTTON_TRIGGER_ACTION_TYPE_FACTORY = 1,
};

enum eventpb_LoggingTypeOptions {
  // [None] logging type "None"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_NONE = 0,
  // [Alive Test Event] logging type "Alive Test Event"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_ALIVE_TEST_EVENT = 1,
  // [Self Test] logging type "Self Test"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_SELF_TEST = 2,
  // [Firmware Update Ok] logging type "Firmware Update Ok"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_FIRMWARE_UPDATE_OK = 10,
  // [Firmware Update Fail] logging type "Firmware Update Fail"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_FIRMWARE_UPDATE_FAIL = 11,
  // [Cold Start] logging type "Cold Start"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_COLD_START = 20,
  // [Warm Start] logging type "Warm Start"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_WARM_START = 21,
  // [Factory Reset] logging type "Factory Reset"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_FACTORY_RESET = 30,
  // [Configuration Loaded] logging type "Configuration Loaded"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_CONFIGURATION_LOADED = 31,
  // [Change Config] logging type "Change Config"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_CHANGE_CONFIG = 32,
  // [Change Offline Config] logging type "Change Offline Config"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_CHANGE_OFFLINE_CONFIG = 33,
  // [Login Success] logging type "Login Success"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_LOGIN_SUCCESS = 40,
  // [Login Password Attempt Fail] logging type "Login Password Attempt Fail"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_LOGIN_PASSWORD_ATTEMPT_FAIL = 41,
  // [Login Interface Access Denied] logging type "Login Interface Access Denied"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_LOGIN_INTERFACE_ACCESS_DENIED = 42,
  // [Login Out] logging type "Login Out"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_LOGIN_OUT = 43,
  // [Link Up] logging type "Link Up"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_LINK_UP = 50,
  // [Link Down] logging type "Link Down"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_LINK_DOWN = 51,
  // [Link State Change] logging type "Link State Change"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_LINK_STATE_CHANGE = 52,
  // [MAC Accepted] logging type "MAC Accepted"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_MAC_ACCEPTED = 60,
  // [MAC Auth Error] logging type "MAC Auth Error"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_MAC_AUTH_ERROR = 61,
  // [MAC Blocked] logging type "MAC Blocked"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_MAC_BLOCKED = 62,
  // [MAC Blocked Vlan] logging type "MAC Blocked Vlan"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_MAC_BLOCKED_VLAN = 63,
  // [MAC Table Change] logging type "MAC Table Change"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_MAC_TABLE_CHANGE = 64,
  // [MAC Auth Request] logging type "MAC Auth Request"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_MAC_AUTH_REQUEST = 65,
  // [MAC Conflict] logging type "MAC Conflict"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_MAC_CONFLICT = 66,
  // [MAC Security Violation] logging type "MAC Security Violation"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_MAC_SECURITY_VIOLATION = 67,
  // [MAC Learning Over Limit] logging type "MAC Learning Over Limit"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_MAC_LEARNING_OVER_LIMIT = 68,
  // [MAC Table Full] logging type "MAC Table Full"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_MAC_TABLE_FULL = 69,
  // [Loop Removed] logging type "Loop Removed"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_LOOP_REMOVED = 80,
  // [Loop Detected] logging type "Loop Detected"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_LOOP_DETECTED = 81,
  // [Lacp Connect] logging type "Lacp Connect"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_LACP_CONNECT = 90,
  // [Lacp Disconnect] logging type "Lacp Disconnect"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_LACP_DISCONNECT = 91,
  // [Ntp Fail] logging type "Ntp Fail"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_NTP_FAIL = 100,
  // [Ntp Sync] logging type "Ntp Sync"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_NTP_SYNC = 101,
  // [Lldp PoE Request] logging type "Lldp PoE Request"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_LLDP_POE_REQUEST = 110,
  // [Packet Intercepted] logging type "Packet Intercepted"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_PACKET_INTERCEPTED = 120,
  // [Network Attack] logging type "Network Attack"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_NETWORK_ATTACK = 121,
  // [CLI Script Execute Success] logging type "CLI Script Execute Success"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_CLI_SCRIPT_EXECUTE_SUCCESS = 130,
  // [CLI Script Execute Fail] logging type "CLI Script Execute Fail"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_CLI_SCRIPT_EXECUTE_FAIL = 131,
  // [Multicast Group Learning Over Limit]
  eventpb_LoggingTypeOptions_LOOING_TYPE_MULTICAST_LEARNING_GROUP_OVER_LIMIT = 140,
  // [SFP Inserted] logging type "SFP Inserted"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_SFP_INSERTED = 200,
  // [SFP Removed] logging type "SFP Removed"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_SFP_REMOVED = 201,
  // [SFP Signal Present] logging type "SFP Signal Present"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_SFP_SIGNAL_PRESENT = 202,
  // [SFP Signal Loss] logging type "SFP Signal Loss"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_SFP_SIGNAL_LOSS = 203,
  // [SFP Signal Change] logging type "SFP Signal Change"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_SFP_SIGNAL_CHANGE = 204,
  // [PoE Connect] logging type "PoE Connect"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_POE_CONNECT = 210,
  // [PoE Voltage] logging type "PoE Voltage"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_POE_VOLTAGE = 211,
  // [PoE Error] logging type "PoE Error"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_POE_ERROR = 212,
  // [PoE Disconnect] logging type "PoE Disconnect"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_POE_DISCONNECT = 213,
  // [PoE Emergency Mode On] logging type "PoE Emergency Mode On"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_POE_EMERGENCY_MODE_ON = 214,
  // [PoE Emergency Mode Off] logging type "PoE Emergency Mode Off"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_POE_EMERGENCY_MODE_OFF = 215,
  // [Led Control] logging type "Led Control"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_LED_CONTROL = 220,
  // [Button Pressed] logging type "Button Pressed"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_BUTTON_PRESSED = 221,
  // [Hardware Error] logging type "Hardware Error"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_HARDWARE_ERROR = 222,
  // [Temperature Ok] logging type "Temperature Ok"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_TEMPERATURE_OK = 230,
  // [Temperature Warning] logging type "Temperature Warning"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_TEMPERATURE_WARNING = 231,
  // [Temperature Failure] logging type "Temperature Failure"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_TEMPERATURE_FAILURE = 232,
  // [CPU Ok] logging type "CPU Ok"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_CPU_OK = 233,
  // [CPU Warning] logging type "CPU Warning"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_CPU_WARNING = 234,
  // [CPU Failure] logging type "CPU Failure"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_CPU_FAILURE = 235,
  // [Memory Ok] logging type "Memory Ok"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_MEMORY_OK = 236,
  // [Memory Warning] logging type "Memory Warning"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_MEMORY_WARNING = 237,
  // [Memory Failure] logging type "Memory Failure"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_MEMORY_FAILURE = 238,
  // [Cable Change Detected] logging type "Cable Change Detected"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_CABLE_CHANGE_DETECTED = 240,
  // [Cable Connection Established] logging type "Cable Connection Established"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_CABLE_CONNECTION_ESTABLISHED = 241,
  // [Cable Connection Lost] logging type "Cable Connection Lost"
  eventpb_LoggingTypeOptions_LOGGING_TYPE_CABLE_CONNECTION_LOST = 242,
};

enum eventpb_CryptoTypeOptions {
  // [Encode]
  eventpb_CryptoTypeOptions_CRYPTO_TYPE_ENCODE = 0,
  // [Decode]
  eventpb_CryptoTypeOptions_CRYPTO_TYPE_DECODE = 1,
  // [MD5]
  eventpb_CryptoTypeOptions_CRYPTO_TYPE_MD5 = 2,
};

enum eventpb_DHCPClientTypeOptions {
  // [V4 Change] V4 Change
  eventpb_DHCPClientTypeOptions_DHCP_CLIENT_TYPE_V4_CHANGE = 0,
  // [V6 Change] V6 Change
  eventpb_DHCPClientTypeOptions_DHCP_CLIENT_TYPE_V6_CHANGE = 1,
};

// for fdb limit over 
enum eventpb_FDBParameterTypeOptions {
  // [Port Over]
  eventpb_FDBParameterTypeOptions_FDB_PARAMETER_TYPE_PORT_OVER = 0,
  // [FDB FUll]
  eventpb_FDBParameterTypeOptions_FDB_PARAMETER_TYPE_FDB_FULL = 1,
};

enum eventpb_ServiceActionTypeOptions {
  // [Start]
  eventpb_ServiceActionTypeOptions_SERVICE_ACTION_TYPE_START = 0,
  // [End]
  eventpb_ServiceActionTypeOptions_SERVICE_ACTION_TYPE_END = 1,
};

enum eventpb_EthernetTypeOptions {
  // [LLC]
  eventpb_EthernetTypeOptions_ETHERNET_TYPE_LLC = 0,
  // [IPv4]
  eventpb_EthernetTypeOptions_ETHERNET_TYPE_IP_V4 = 2048,
  // [ARP]
  eventpb_EthernetTypeOptions_ETHERNET_TYPE_ARP = 2054,
  // [IPv6]
  eventpb_EthernetTypeOptions_ETHERNET_TYPE_IP_V6 = 34525,
  // [Cisco Discovery]
  eventpb_EthernetTypeOptions_ETHERNET_TYPE_CISCO_DISCOVERY = 8192,
  // [Dot1Q]
  eventpb_EthernetTypeOptions_ETHERNET_TYPE_DOT_1Q = 33024,
  // [Lacp]
  eventpb_EthernetTypeOptions_ETHERNET_TYPE_LACP = 34825,
  // [EAPOL]
  eventpb_EthernetTypeOptions_ETHERNET_TYPE_EAPOL = 34958,
  // [Link Layer Discovery]
  eventpb_EthernetTypeOptions_ETHERNET_TYPE_LINK_LAYER_DISCOVERY = 35020,
  // [IEEE 1588]
  eventpb_EthernetTypeOptions_ETHERNET_TYPE_IEEE_1588 = 35063,
  // [User Defined]
  eventpb_EthernetTypeOptions_ETHERNET_TYPE_USER_DEFINED = 65535,
};

enum eventpb_IPProtocolTypeOptions {
  // [IPv6 HopByHop]
  eventpb_IPProtocolTypeOptions_IP_PROTOCOL_TYPE_IP_V6_HOPBYHOP = 0,
  // [IGMP]
  eventpb_IPProtocolTypeOptions_IP_PROTOCOL_TYPE_IGMP = 2,
  // [IPv4]
  eventpb_IPProtocolTypeOptions_IP_PROTOCOL_TYPE_IP_V4 = 4,
  // [TCP]
  eventpb_IPProtocolTypeOptions_IP_PROTOCOL_TYPE_TCP = 6,
  // [UDP]
  eventpb_IPProtocolTypeOptions_IP_PROTOCOL_TYPE_UDP = 17,
  // [IPv6]
  eventpb_IPProtocolTypeOptions_IP_PROTOCOL_TYPE_IP_V6 = 41,
};

enum eventpb_LinkTypeOptions {
  // [Up] Link up
  eventpb_LinkTypeOptions_LINK_TYPE_UP = 0,
  // [Down] Link down
  eventpb_LinkTypeOptions_LINK_TYPE_DOWN = 1,
};

enum eventpb_LoginResultTypeOptions {
  // [Success] login success
  eventpb_LoginResultTypeOptions_LOGIN_RESULT_TYPE_SUCCESS = 0,
  // [Loading Access Token] loading access token
  eventpb_LoginResultTypeOptions_LOGIN_RESULT_TYPE_LOADING_ACCESS_TOKEN = 1,
  // [Password Attempt Failed] login failed due to wrong password
  eventpb_LoginResultTypeOptions_LOGIN_RESULT_TYPE_PASSWORD_ATTEMPT_FAILED = 2,
  // [Interface Access Denied] the interface is forbidden for the current user login
  eventpb_LoginResultTypeOptions_LOGIN_RESULT_TYPE_INTERFACE_ACCESS_DENIED = 3,
  // [Attempt Failed] login attempt failed
  eventpb_LoginResultTypeOptions_LOGIN_RESULT_TYPE_ATTEMPT_FAILED = 4,
};

enum eventpb_LoginTypeOptions {
  // [Login Local] local user login
  eventpb_LoginTypeOptions_LOGIN_TYPE_LOGIN_LOCAL = 0,
  // [Login Radius] radius user account login
  eventpb_LoginTypeOptions_LOGIN_TYPE_LOGIN_RADIUS = 1,
  // [Login TACACS+] TACACS+ user account login
  eventpb_LoginTypeOptions_LOGIN_TYPE_LOGIN_TACPLUS = 2,
  // [PACC Via MAC Table] PACC Via MAC Table
  eventpb_LoginTypeOptions_LOGIN_TYPE_PACC_VIA_MAC_TABLE = 3,
  // [PACC Mac Via Radius] PACC Mac Via Radius
  eventpb_LoginTypeOptions_LOGIN_TYPE_PACC_MAC_VIA_RADIUS = 4,
  // [PACC 802.1X Via Radius] PACC 802.1X Via Radius
  eventpb_LoginTypeOptions_LOGIN_TYPE_PACC_802_1X_VIA_RADIUS = 5,
  // [PACC Via Mac Event Only] PACC Via Mac Event Only
  eventpb_LoginTypeOptions_LOGIN_TYPE_PACC_VIA_MAC_EVENT_ONLY = 6,
  // [PACC Edge 802.1X Via Radius] PACC Edge 802.1X Via Radius
  eventpb_LoginTypeOptions_LOGIN_TYPE_PACC_EDGE_802_1X_VIA_RADIUS = 7,
};

enum eventpb_LoginInterfaceNameTypeOptions {
  // [SSH] user login through SSH connection
  eventpb_LoginInterfaceNameTypeOptions_LOGIN_INTERFACE_NAME_TYPE_SSH = 0,
  // [Telnet] user login through Telnet connection
  eventpb_LoginInterfaceNameTypeOptions_LOGIN_INTERFACE_NAME_TYPE_TELNET = 1,
  // [WEB] user login through WEB connection
  eventpb_LoginInterfaceNameTypeOptions_LOGIN_INTERFACE_NAME_TYPE_WEB = 2,
  // [SNMP] user login through SNMP connection
  eventpb_LoginInterfaceNameTypeOptions_LOGIN_INTERFACE_NAME_TYPE_SNMP = 3,
  // [FTP] user login through FTP connection
  eventpb_LoginInterfaceNameTypeOptions_LOGIN_INTERFACE_NAME_TYPE_FTP = 4,
  // [Console] user login through Console connection
  eventpb_LoginInterfaceNameTypeOptions_LOGIN_INTERFACE_NAME_TYPE_CONSOLE = 5,
};

enum eventpb_MaintenanceActionTypeOptions {
  // [Snapshot] Maintenance action type snapshot
  eventpb_MaintenanceActionTypeOptions_MAINTENANCE_ACTION_TYPE_SNAPSHOT = 0,
  // [Firmware Upgrade Fail] Maintenance action type firmware upgrade fail
  eventpb_MaintenanceActionTypeOptions_MAINTENANCE_ACTION_TYPE_FIRMWARE_UPGRADE_FAIL = 1,
  // [Firmware Upgrade Success] Maintenance action type firmware upgrade success
  eventpb_MaintenanceActionTypeOptions_MAINTENANCE_ACTION_TYPE_FIRMWARE_UPGRADE_SUCCESS = 2,
  // [Config Import] Maintenance action type config import
  eventpb_MaintenanceActionTypeOptions_MAINTENANCE_ACTION_TYPE_CONFIG_IMPORT = 3,
  // [Config Export] Maintenance action type config export
  eventpb_MaintenanceActionTypeOptions_MAINTENANCE_ACTION_TYPE_CONFIG_EXPORT = 4,
};

enum eventpb_MulticastActionTypeOptions {
  // [Group Over Limit]
  eventpb_MulticastActionTypeOptions_MULTICAST_ACTION_TYPE_LEARNING_GROUP_OVER_LIMIT = 0,
};

enum eventpb_PoEParameterTypeOptions {
  // [Budget MAX]
  eventpb_PoEParameterTypeOptions_POE_PARAMETER_TYPE_BUDGET_MAX = 0,
  // [Budget Enough]
  eventpb_PoEParameterTypeOptions_POE_PARAMETER_TYPE_BUDGET_ENOUGH = 1,
  // [PoE Connect]
  eventpb_PoEParameterTypeOptions_POE_PARAMETER_TYPE_POE_CONNECT = 2,
  // [PoE Disconnect]
  eventpb_PoEParameterTypeOptions_POE_PARAMETER_TYPE_POE_DISCONNECT = 3,
  // [PoE Error]
  eventpb_PoEParameterTypeOptions_POE_PARAMETER_TYPE_POE_ERROR = 4,
};

enum eventpb_PortSpeedDuplexTypeOptions {
  // [Auto]
  eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_AUTO = 0,
  // [10 Mbps / Full]
  eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_10M_FULL = 1,
  // [100 Mbps / Full]
  eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_100M_FULL = 3,
  // [1 Gbps  / Full]
  eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_1000M_FULL = 5,
  // [2.5 Gbps / Full]
  eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_2500M_FULL = 6,
  // [5 Gbps / Full]
  eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_5G_FULL = 7,
  // [10 Gbps / Full]
  eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_10G_FULL = 8,
  // [25 Gbps / Full]
  eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_25G_FULL = 9,
  // [40 Gbps / Full]
  eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_40G_FULL = 10,
  // [100 Gbps / Full]
  eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_100G_FULL = 11,
  // [NA]
  eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_NA = 12,
};

enum eventpb_SFPActionTypeOptions {
  // [Inserted] Inserted
  eventpb_SFPActionTypeOptions_SFP_ACTION_TYPE_INSERTED = 0,
  // [Removed] Removed
  eventpb_SFPActionTypeOptions_SFP_ACTION_TYPE_REMOVED = 1,
  // [Signal Fault] Signal fault
  eventpb_SFPActionTypeOptions_SFP_ACTION_TYPE_SIGNAL_FAULT = 2,
  // [Signal Loss] Signal loss
  eventpb_SFPActionTypeOptions_SFP_ACTION_TYPE_SIGNAL_LOSS = 3,
};

enum eventpb_SFPStatusTypeOptions {
  // [Unknown] This is show when no data could be retrieved.
  eventpb_SFPStatusTypeOptions_SFP_STATUS_TYPE_UNKNOWN = 0,
  // [Ok] Optical operation conditions are OK.
  eventpb_SFPStatusTypeOptions_SFP_STATUS_TYPE_OK = 1,
  // [Disabled] Laser is disabled. No data can be sent. The port may be disabled.
  eventpb_SFPStatusTypeOptions_SFP_STATUS_LASER_TYPE_DISABLED = 2,
  // [Loss Of Signal] This flag is set when the optical receive power level is below the critical lower limit.
  eventpb_SFPStatusTypeOptions_SFP_STATUS_TYPE_LOSS_OF_SIGNAL = 3,
  // [Read Error] Management read access to the SFP has failed.
  eventpb_SFPStatusTypeOptions_SFP_STATUS_TYPE_READ_ERROR = 4,
};

enum eventpb_SFPTypeOptions {
  // [Empty] No SFP is inserted.
  eventpb_SFPTypeOptions_SFP_TYPE_EMPTY = 0,
  // [Unknown] An SFP is inserted but its type could not be decoded.
  eventpb_SFPTypeOptions_SFP_TYPE_UNKNOWN = 1,
  // [Or Plus] A normal SFP or SFP+ is inserted.
  eventpb_SFPTypeOptions_SFP_TYPE_OR_PLUS = 2,
  // [Gbic] A GBIC is inserted.
  eventpb_SFPTypeOptions_SFP_TYPE_GBIC = 3,
  // [Sff] An SFF fixed optical interface is installed.
  eventpb_SFPTypeOptions_SFP_TYPE_SFF = 4,
};

enum eventpb_SFPConnectorTypeOptions {
  // [Unknown] Connector type cannot be decoded.
  eventpb_SFPConnectorTypeOptions_SFP_CONNECTOR_TYPE_UNKNOWN = 0,
  // [Lc] LC connector is used.
  eventpb_SFPConnectorTypeOptions_SFP_CONNECTOR_TYPE_LC = 1,
  // [Sc] SC connector is used.
  eventpb_SFPConnectorTypeOptions_SFP_CONNECTOR_TYPE_SC = 2,
  // [Mt Rj] MT_RJ connector is used.
  eventpb_SFPConnectorTypeOptions_SFP_CONNECTOR_TYPE_MT_RJ = 3,
  // [Rj45] Electrical RJ45 connector is used.
  eventpb_SFPConnectorTypeOptions_SFP_CONNECTOR_TYPE_RJ45 = 4,
  // [Mu] MU connector is used.
  eventpb_SFPConnectorTypeOptions_SFP_CONNECTOR_TYPE_MU = 5,
};

enum eventpb_StormcontrolActionTypeOptions {
  // [Normal] the port is normal
  eventpb_StormcontrolActionTypeOptions_STORMCONTROL_ACTION_TYPE_PORT_NORMAL = 0,
  // [Shutdown] the port is shutdown
  eventpb_StormcontrolActionTypeOptions_STORMCONTROL_ACTION_TYPE_PORT_SHUTDOWN = 1,
  // [Blocking] the port is blocking
  eventpb_StormcontrolActionTypeOptions_STORMCONTROL_ACTION_TYPE_PORT_BLOCKING = 2,
};

enum eventpb_NTPActionTypeOptions {
  // [Fail] Fail
  eventpb_NTPActionTypeOptions_NTP_ACTION_TYPE_FAIL = 0,
  // [Sync] Sync
  eventpb_NTPActionTypeOptions_NTP_ACTION_TYPE_SYNC = 1,
};

enum eventpb_BootActionTypeOptions {
  // [Warm Start] Warm Start
  eventpb_BootActionTypeOptions_BOOT_ACTION_TYPE_WARM_START = 0,
  // [Cold Start] Cold Start
  eventpb_BootActionTypeOptions_BOOT_ACTION_TYPE_COLD_START = 1,
  // [Ready] Ready
  eventpb_BootActionTypeOptions_BOOT_ACTION_TYPE_READY = 2,
  // [Port Ready] Port Ready
  eventpb_BootActionTypeOptions_BOOT_ACTION_TYPE_PORT_READY = 3,
};

enum eventpb_NetworkIPv6TypeOptions {
  // [Link Local] Ipv6 type link local
  eventpb_NetworkIPv6TypeOptions_IPV6_TYPE_LINK_LOCAL = 0,
  // [Virtual Link Local] Ipv6 type virtual link local
  eventpb_NetworkIPv6TypeOptions_IPV6_TYPE_VIRTUAL_LINK_LOCAL = 1,
  // [Unicast] Ipv6 type unicast
  eventpb_NetworkIPv6TypeOptions_IPV6_TYPE_UNICAST = 2,
};

enum eventpb_UdldActionTypeOptions {
  // [Shutdown] the port is shutdown
  eventpb_UdldActionTypeOptions_UDLD_ACTION_PORT_SHUTDOWN = 0,
};

// for vlan changed
enum eventpb_VLANParameterTypeOptions {
  // [Managment Add]
  eventpb_VLANParameterTypeOptions_VLAN_PARAMETER_TYPE_MANAGMENT_ADD = 0,
  // [Managment Delete]
  eventpb_VLANParameterTypeOptions_VLAN_PARAMETER_TYPE_MANAGMENT_DELETE = 1,
  // [UnAuth Change]
  eventpb_VLANParameterTypeOptions_VLAN_PARAMETER_TYPE_UNAUTH_VLAN_CHANGE = 2,
};

enum eventpb_ListenTypeOptions {
  // [Tx] Listen type tx
  eventpb_ListenTypeOptions_LISTEN_TYPE_TX = 0,
  // [Rx] Listen type rx
  eventpb_ListenTypeOptions_LISTEN_TYPE_RX = 1,
  // [Both] Listen type both
  eventpb_ListenTypeOptions_LISTEN_TYPE_BOTH = 2,
};

enum eventpb_InternalTypeOptions {
  // [All] Internal type all
  eventpb_InternalTypeOptions_INTERNAL_TYPE_ALL = 0,
  // [Service] Internal type service
  eventpb_InternalTypeOptions_INTERNAL_TYPE_SERVICE = 1,
  // [Hardware] Internal type hardware
  eventpb_InternalTypeOptions_INTERNAL_TYPE_HARDWARE = 10,
  // [Led] Internal type led
  eventpb_InternalTypeOptions_INTERNAL_TYPE_LED = 11,
  // [Poe] Internal type poe
  eventpb_InternalTypeOptions_INTERNAL_TYPE_POE = 12,
  // [Sfp] Internal type sfp
  eventpb_InternalTypeOptions_INTERNAL_TYPE_SFP = 13,
  // [Cable] Internal type cable
  eventpb_InternalTypeOptions_INTERNAL_TYPE_CABLE = 14,
  // [Port] Internal type port
  eventpb_InternalTypeOptions_INTERNAL_TYPE_PORT = 15,
  // [Button] Internal type button
  eventpb_InternalTypeOptions_INTERNAL_TYPE_BUTTON = 16,
  // [Login] Internal type login
  eventpb_InternalTypeOptions_INTERNAL_TYPE_LOGIN = 20,
  // [Script] Internal type script
  eventpb_InternalTypeOptions_INTERNAL_TYPE_SCRIPT = 21,
  // [System] Internal type system
  eventpb_InternalTypeOptions_INTERNAL_TYPE_SYSTEM = 22,
  // [Maintenance] Internal type maintenance
  eventpb_InternalTypeOptions_INTERNAL_TYPE_MAINTENANCE = 23,
  // [Ntp] Internal type ntp
  eventpb_InternalTypeOptions_INTERNAL_TYPE_NTP = 24,
  // [Link] Internal type link
  eventpb_InternalTypeOptions_INTERNAL_TYPE_LINK = 25,
  // [Network] Internal type network
  eventpb_InternalTypeOptions_INTERNAL_TYPE_NETWORK = 26,
  // [Boot] Internal type boot
  eventpb_InternalTypeOptions_INTERNAL_TYPE_BOOT = 27,
  // [Monitor] Internal type monitor
  eventpb_InternalTypeOptions_INTERNAL_TYPE_MONITOR = 28,
  // [Portauth] Internal type portauth
  eventpb_InternalTypeOptions_INTERNAL_TYPE_PORTAUTH = 40,
  // [G8032 Ring] Internal type g8032 ring
  eventpb_InternalTypeOptions_INTERNAL_TYPE_G8032RING = 41,
  // [Aggregation] Internal type aggregation
  eventpb_InternalTypeOptions_INTERNAL_TYPE_AGGREGATION = 42,
  // [Loop] Internal type loop
  eventpb_InternalTypeOptions_INTERNAL_TYPE_LOOP = 43,
  // [Lldp] Internal type lldp
  eventpb_InternalTypeOptions_INTERNAL_TYPE_LLDP = 44,
  // [Cdp] Internal type cdp
  eventpb_InternalTypeOptions_INTERNAL_TYPE_CDP = 45,
  // [Vlan] Internal type vlan
  eventpb_InternalTypeOptions_INTERNAL_TYPE_VLAN = 46,
  // [Dhcp] Internal type dhcp
  eventpb_InternalTypeOptions_INTERNAL_TYPE_DHCP = 47,
  // [Igmp] Internal type igmp
  eventpb_InternalTypeOptions_INTERNAL_TYPE_IGMP = 48,
  // [Mld] Internal type mld
  eventpb_InternalTypeOptions_INTERNAL_TYPE_MLD = 49,
  // [Fdb] Internal type fdb
  eventpb_InternalTypeOptions_INTERNAL_TYPE_FDB = 50,
  // [Au] Internal type au
  eventpb_InternalTypeOptions_INTERNAL_TYPE_AU = 51,
  // [Acl] Internal type acl
  eventpb_InternalTypeOptions_INTERNAL_TYPE_ACL = 52,
  // [Timerange] Internal type timerange
  eventpb_InternalTypeOptions_INTERNAL_TYPE_TIMERANGE = 53,
  // [Stormcontrol] Internal type stormcontrol
  eventpb_InternalTypeOptions_INTERNAL_TYPE_STORMCONTROL = 54,
  // [Udld] Internal type UDLD
  eventpb_InternalTypeOptions_INTERNAL_TYPE_UDLD = 55,
  // [Test] Internal type test
  eventpb_InternalTypeOptions_INTERNAL_TYPE_TEST = 90,
  // [Api] Internal type api
  eventpb_InternalTypeOptions_INTERNAL_TYPE_API = 91,
  // [Debug] Internal type debug
  eventpb_InternalTypeOptions_INTERNAL_TYPE_DEBUG = 99,
};

enum eventpb_TargetLogTypeOptions {
  // [Disabled] This logging entry is disabled
  eventpb_TargetLogTypeOptions_TARGET_LOG_TYPE_DISABLED = 0,
  // [Syslog] The event will be forwarded as Syslog
  eventpb_TargetLogTypeOptions_TARGET_LOG_TYPE_SYSLOG = 1,
  // [Snmp Trap V1] The event will be forwarded as SNMP trap using SNMP V1 format
  eventpb_TargetLogTypeOptions_TARGET_LOG_TYPE_SNMP_TRAP_V1 = 2,
  // [Snmp Trap V2C] The event will be forwarded as SNMP trap using SNMP V2c format
  eventpb_TargetLogTypeOptions_TARGET_LOG_TYPE_SNMP_TRAP_V2C = 3,
  // [Snmp Trap V3] The event will be forwarded as SNMP trap using SNMP V3 format
  eventpb_TargetLogTypeOptions_TARGET_LOG_TYPE_SNMP_TRAP_V3 = 4,
  // [Snmp Inform V2 C] The event will be forwarded as SNMP trap using SNMP V2c acknowledged Inform format
  eventpb_TargetLogTypeOptions_TARGET_LOG_TYPE_SNMP_INFORM_V2C = 5,
  // [Snmp Inform V3] The event will be forwarded as SNMP trap using SNMP V3 acknowledged Inform format
  eventpb_TargetLogTypeOptions_TARGET_LOG_TYPE_SNMP_INFORM_V3 = 6,
  // [Display In Cli] The event will be shown to all currently open terminal sessions
  eventpb_TargetLogTypeOptions_TARGET_LOG_TYPE_DISPLAY_IN_CLI = 7,
  // [Display In Web] The event will be shown to all currently open web sessions
  eventpb_TargetLogTypeOptions_TARGET_LOG_TYPE_DISPLAY_IN_WEB = 9,
};

enum eventpb_LoggingSeverityTypeOptions {
  // [Disabled] Syslog output to this target is disabled
  eventpb_LoggingSeverityTypeOptions_LOGGING_SEVERITY_TYPE_DISABLED = 0,
  // [Debug] Internal system debugging information
  eventpb_LoggingSeverityTypeOptions_LOGGING_SEVERITY_TYPE_DEBUG = 1,
  // [Info] Information with no important consequences
  eventpb_LoggingSeverityTypeOptions_LOGGING_SEVERITY_TYPE_INFO = 2,
  // [Notice] Notification about normal occurrence
  eventpb_LoggingSeverityTypeOptions_LOGGING_SEVERITY_TYPE_NOTICE = 3,
  // [Warning] Warning about a normal problem
  eventpb_LoggingSeverityTypeOptions_LOGGING_SEVERITY_TYPE_WARNING = 4,
  // [Error] Unexpected error has occurred
  eventpb_LoggingSeverityTypeOptions_LOGGING_SEVERITY_TYPE_ERROR = 5,
  // [Critical] Critical error which compromises data traffic or stability
  eventpb_LoggingSeverityTypeOptions_LOGGING_SEVERITY_TYPE_CRITICAL = 6,
  // [Alert] Very important error condition
  eventpb_LoggingSeverityTypeOptions_LOGGING_SEVERITY_TYPE_ALERT = 7,
  // [Emergency] Highest possible error condition (no set by this product)
  eventpb_LoggingSeverityTypeOptions_LOGGING_SEVERITY_TYPE_EMERGENCY = 8,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct eventpb_ACLParameter {
  char *Name;
  int32_t Index;
  enum eventpb_ACLActionTypeOptions Type;
};

struct eventpb_LgportUsed {
  enum eventpb_AggrTypeOptions Action;
  unsigned int OriginalMembers_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **OriginalMembers;
  unsigned int NewMembers_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **NewMembers;
  unsigned int AddedMembers_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **AddedMembers;
  unsigned int DeletedMembers_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **DeletedMembers;
};

struct eventpb_ProtocolUsed {
  unsigned int UpdateMembers_Len; // auto-gen: for list
  struct devicepb_InterfaceIdentify **UpdateMembers;
};

// for aggregation changed event message
enum eventpb_AggrParameter_Used_Union_Options {
  eventpb_AggrParameter_Used_Union_Options_Lgport,
  eventpb_AggrParameter_Used_Union_Options_Protocol,
};
struct eventpb_AggrParameter {
  enum eventpb_AggrSourceTypeOptions Type;
  int32_t TrunkID;
  enum eventpb_AggrParameter_Used_Union_Options Used_Union_Option;
  union {
    struct eventpb_LgportUsed *Used_Lgport;
    struct eventpb_ProtocolUsed *Used_Protocol;
  } Used;
};

struct eventpb_AUParameter {
  enum eventpb_AUParameterTypeOptions Type;
  struct eventpb_FDBEntry *Entry;
};

struct eventpb_FDBEntry {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  bool IsStatic;
  bool IsForward;
  char *Address;
  int32_t VlanID;
  bool IsAgeout;
  enum eventpb_FDBEntryActionTypeOptions Action;
};

// for vlan changed event message
struct eventpb_ButtonParameter {
  enum eventpb_ButtonTypeOptions Type;
  enum eventpb_ButtonActionTypeOptions Action;
  enum eventpb_ButtonTriggerActionTypeOptions Trigger;
};

struct eventpb_CryptoRequest {
  char *Value;
  enum eventpb_CryptoTypeOptions Type;
};

struct eventpb_CryptoResponse {
  char *Value;
};

struct eventpb_CryptoBase64Request {
  unsigned char *Value;
};

struct eventpb_CryptoBase64Response {
  unsigned char *Value;
};

struct eventpb_DHCPParameter {
  char *TFTPServer;
  char *BootFile;
  enum eventpb_DHCPClientTypeOptions Type;
};

// for vlan changed event message
struct eventpb_FDBParameter {
  enum eventpb_FDBParameterTypeOptions Type;
  struct devicepb_InterfaceIdentify *Port;
};

struct eventpb_ServiceInitialized {
  // Reference intri-utils/resources/services_enum.go
  char *ServiceType;
  enum eventpb_ServiceActionTypeOptions Action;
};

struct eventpb_EthernetLayer {
  unsigned char *DstMACAddr;
  bool DstMACAddrEnabled;
  unsigned char *SrcMACAddr;
  bool SrcMACAddrEnabled;
  enum eventpb_EthernetTypeOptions EtherType;
  bool EtherTypeEnabled;
};

struct eventpb_LLCLayer {
  uint32_t DSAP;
  bool DSAPEnabled;
  bool IG;
  bool IGEnabled;
  uint32_t SSAP;
  bool SSAPEnabled;
  bool CR;
  bool CREnabled;
  uint32_t Control;
  bool ControlEnabled;
};

struct eventpb_IPLayer {
  enum eventpb_IPProtocolTypeOptions IPProtocol;
  bool IPProtocolEnabled;
  unsigned char *SrcIPAddr;
  bool SrcIPAddrEnabled;
  unsigned char *DstIPAddr;
  bool DstIPAddrEnabled;
};

struct eventpb_Layer4Port {
  uint32_t L4SrcPort;
  bool L4SrcPortEnabled;
  uint32_t L4DstPort;
  bool L4DstPortEnabled;
};

struct eventpb_LinkParameter {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  enum eventpb_LinkTypeOptions Type;
};

struct eventpb_LoginParameter {
  enum eventpb_LoginTypeOptions Type;
  enum eventpb_LoginResultTypeOptions Result;
  enum eventpb_LoginInterfaceNameTypeOptions InterfaceName;
  char *Name;
  int32_t Privilege;
  char *Token;
  char *AccessToken;
  char *ErrCode;
  char *IPAddress;
};

struct eventpb_MaintenanceParameter {
  enum eventpb_MaintenanceActionTypeOptions Action;
};

struct eventpb_MulticastParameter {
  enum eventpb_MulticastActionTypeOptions Type;
  int32_t VlanID;
  char *Addr;
};

struct eventpb_PacketProbe {
  unsigned int List_Len; // auto-gen: for list
  struct eventpb_PacketProbeEntry **List;
  // false : receive packets from physical ports
// true : receive packets from physical ports and logical ports(LAGs)
  bool IsPhysicalPort;
};

struct eventpb_PacketProbeEntry {
  struct eventpb_EthernetLayer *Ethernet;
  bool EthernetEnabled;
  struct eventpb_LLCLayer *LLC;
  bool LLCEnabled;
  struct eventpb_IPLayer *IP;
  bool IPEnabled;
  struct eventpb_Layer4Port *L4;
  bool L4Enabled;
};

struct eventpb_PacketInfo {
  struct devicepb_InterfaceIdentify *Identify;
  bool Forward;
  // when forward is true, vlan_tagged can be ignored
// when forward is false, if the vlan_tagged is true, the packet will be transmitted to the tagged VLAN
  bool VlanTagged;
  // if true, the packet will be affected by blocking and vlan
  bool EgressFilter;
  // when forward is true, exclude_identify should not receive this packet again
  struct devicepb_InterfaceIdentify *ExcludeIdentify;
};

struct eventpb_PacketContent {
  struct eventpb_PacketInfo *Info;
  unsigned char *Buf;
};

struct eventpb_PoEParameter {
  enum eventpb_PoEParameterTypeOptions Type;
  int32_t PortNo;
};

struct eventpb_PortParameter {
  int32_t DeviceID;
  int32_t PortNo;
  bool Enabled;
};

struct eventpb_PortAuthParameter {
  struct devicepb_InterfaceIdentify *IdentifyNo;
  int32_t VlanID;
  char *MACAddress;
};

struct eventpb_SFPInfo {
  int32_t DeviceID;
  int32_t PortNo;
  char *Location;
  unsigned int Status_Len; // auto-gen: for list
  enum eventpb_SFPStatusTypeOptions *Status;
  enum eventpb_SFPTypeOptions Type;
  enum eventpb_SFPConnectorTypeOptions Connector;
  char *Wavelength;
  char *TxTechnology;
  char *RxTechnology;
  char *NominalBitrate;
  char *Manufacturer;
  char *PartNumber;
  char *Revision;
  char *SerialNumber;
  char *MfgDateCode;
  char *TxPower;
  char *RxPower;
  char *Temperature;
  char *MaxLength_9Um;
  char *MaxLength_50Um;
  char *MaxLength_62Um;
  char *MaxLengthCopper;
  char *TuningRange;
  char *PowerConsumption;
  unsigned int AdditionalInformation_Len; // auto-gen: for list
  char **AdditionalInformation;
  enum eventpb_PortSpeedDuplexTypeOptions Speed;
};

struct eventpb_SFPParameter {
  enum eventpb_SFPActionTypeOptions Type;
  int32_t DeviceID;
  int32_t PortNo;
  // Only for Inserted 
  struct eventpb_SFPInfo *Info;
};

struct eventpb_StormcontrolParameter {
  enum eventpb_StormcontrolActionTypeOptions ActionOption;
  struct devicepb_InterfaceIdentify *IdentifyNo;
};

struct eventpb_BootParameter {
  enum eventpb_BootActionTypeOptions Type;
  char *Version;
};

struct eventpb_NetworkParameter {
  struct devicepb_InterfaceIdentify *Inf;
  char *IPAddress;
  char *SubnetMask;
  enum eventpb_NetworkIPv6TypeOptions Type;
};

// for time range active
struct eventpb_TimeRangeParameter {
  char *Name;
  bool IsActive;
};

enum eventpb_UdldParameter_ActionOptionParam_Union_Options {
  eventpb_UdldParameter_ActionOptionParam_Union_Options_PortShutdown,
};
struct eventpb_UdldParameter {
  enum eventpb_UdldActionTypeOptions ActionOption;
  enum eventpb_UdldParameter_ActionOptionParam_Union_Options ActionOptionParam_Union_Option;
  union {
    struct eventpb_UdldActionPortShutdown *ActionOptionParam_PortShutdown;
  } ActionOptionParam;
};

struct eventpb_UdldActionPortShutdown {
  struct devicepb_InterfaceIdentify *IdentifyNo;
};

// for vlan changed event message
struct eventpb_VLANParameter {
  enum eventpb_VLANParameterTypeOptions Type;
  int32_t VlanID;
  int32_t DeviceID;
  struct devicepb_InterfaceIdentify *IdentifyNo;
};

struct eventpb_ManagmentVLANPriority {
  int32_t Priority;
};

struct eventpb_InternalTypeUnion {
  unsigned int List_Len; // auto-gen: for list
  enum eventpb_InternalTypeOptions *List;
};

enum eventpb_Internal_Parameter_Union_Options {
  eventpb_Internal_Parameter_Union_Options_Init,
  eventpb_Internal_Parameter_Union_Options_Vlan,
  eventpb_Internal_Parameter_Union_Options_DHCP,
  eventpb_Internal_Parameter_Union_Options_Login,
  eventpb_Internal_Parameter_Union_Options_Link,
  eventpb_Internal_Parameter_Union_Options_Aggr,
  eventpb_Internal_Parameter_Union_Options_NTP,
  eventpb_Internal_Parameter_Union_Options_Boot,
  eventpb_Internal_Parameter_Union_Options_Network,
  eventpb_Internal_Parameter_Union_Options_Maintenance,
  eventpb_Internal_Parameter_Union_Options_SFP,
  eventpb_Internal_Parameter_Union_Options_Port,
  eventpb_Internal_Parameter_Union_Options_PortAuth,
  eventpb_Internal_Parameter_Union_Options_FDB,
  eventpb_Internal_Parameter_Union_Options_AU,
  eventpb_Internal_Parameter_Union_Options_PoE,
  eventpb_Internal_Parameter_Union_Options_ACL,
  eventpb_Internal_Parameter_Union_Options_TimeRange,
  eventpb_Internal_Parameter_Union_Options_Button,
  eventpb_Internal_Parameter_Union_Options_Stormcontrol,
  eventpb_Internal_Parameter_Union_Options_Multicast,
  eventpb_Internal_Parameter_Union_Options_Udld,
};
struct eventpb_Internal {
  enum eventpb_InternalTypeOptions Type;
  char *Message;
  struct timestamppb_Timestamp *Ts;
  enum eventpb_LoggingTypeOptions LoggingType;
  enum eventpb_Internal_Parameter_Union_Options Parameter_Union_Option;
  union {
    struct eventpb_ServiceInitialized *Parameter_Init;
    struct eventpb_VLANParameter *Parameter_Vlan;
    struct eventpb_DHCPParameter *Parameter_DHCP;
    struct eventpb_LoginParameter *Parameter_Login;
    struct eventpb_LinkParameter *Parameter_Link;
    struct eventpb_AggrParameter *Parameter_Aggr;
    enum eventpb_NTPActionTypeOptions Parameter_NTP;
    struct eventpb_BootParameter *Parameter_Boot;
    struct eventpb_NetworkParameter *Parameter_Network;
    struct eventpb_MaintenanceParameter *Parameter_Maintenance;
    struct eventpb_SFPParameter *Parameter_SFP;
    struct eventpb_PortParameter *Parameter_Port;
    struct eventpb_PortAuthParameter *Parameter_PortAuth;
    struct eventpb_FDBParameter *Parameter_FDB;
    struct eventpb_AUParameter *Parameter_AU;
    struct eventpb_PoEParameter *Parameter_PoE;
    struct eventpb_ACLParameter *Parameter_ACL;
    struct eventpb_TimeRangeParameter *Parameter_TimeRange;
    struct eventpb_ButtonParameter *Parameter_Button;
    struct eventpb_StormcontrolParameter *Parameter_Stormcontrol;
    struct eventpb_MulticastParameter *Parameter_Multicast;
    struct eventpb_UdldParameter *Parameter_Udld;
  } Parameter;
};

struct eventpb_TargetLogTypeUnion {
  unsigned int List_Len; // auto-gen: for list
  enum eventpb_TargetLogTypeOptions *List;
};

struct eventpb_TargetSNMP {
  char *HostAddress;
  char *SnmpCommunity;
  char *SnmpV3Username;
};

struct eventpb_TargetCLI {
};

struct eventpb_TargetSysLog {
  char *HostAddress;
};

enum eventpb_TargetLogOptionalParameter_OptionParam_Union_Options {
  eventpb_TargetLogOptionalParameter_OptionParam_Union_Options_Snmp,
  eventpb_TargetLogOptionalParameter_OptionParam_Union_Options_Cli,
  eventpb_TargetLogOptionalParameter_OptionParam_Union_Options_SysLog,
};
struct eventpb_TargetLogOptionalParameter {
  enum eventpb_TargetLogTypeOptions Option;
  enum eventpb_TargetLogOptionalParameter_OptionParam_Union_Options OptionParam_Union_Option;
  union {
    struct eventpb_TargetSNMP *OptionParam_Snmp;
    struct eventpb_TargetCLI *OptionParam_Cli;
    struct eventpb_TargetSysLog *OptionParam_SysLog;
  } OptionParam;
};

struct eventpb_TargetLog {
  uint64_t LogID;
  struct timestamppb_Timestamp *Ts;
  enum eventpb_LoggingTypeOptions LoggingType;
  enum eventpb_LoggingSeverityTypeOptions LoggingSeverityType;
  char *Message;
  struct eventpb_TargetLogOptionalParameter *Param;
  struct eventpb_Internal *Internal;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_event_event
