
#ifndef _H_intri_pb_github_com_Intrising_intri_type_error_error
#define _H_intri_pb_github_com_Intrising_intri_type_error_error

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include <stdbool.h>
#include <stdint.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum errorpb_ErrorTypeOptions {
  // unexpected error
  errorpb_ErrorTypeOptions_ERROR_TYPE_UNKNOWN = 0,
  // errpr from other services
  errorpb_ErrorTypeOptions_ERROR_TYPE_OTHER_SERVICES = 1,
  // the %s must contain between %s and %s items, inclusive
  errorpb_ErrorTypeOptions_ERROR_TYPE_LIST_FULL = 2,
  // the ip address %s is unreachable
  errorpb_ErrorTypeOptions_ERROR_TYPE_UNREACHABLE = 3,
  // configuration is empty
  errorpb_ErrorTypeOptions_ERROR_TYPE_CONFIG_EMPTY = 10,
  // the interface %s tagged list should be empty when the mode is %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_TAGGED_LIST_SHOULD_BE_EMPTY = 11,
  // the interface %s untagged list should be empty when the mode is %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_UNTAGGED_LIST_SHOULD_BE_EMPTY = 12,
  // the DNS name cannot be empty
  errorpb_ErrorTypeOptions_ERROR_TYPE_DIAGNOSTIC_DNS_EMPTY = 13,
  // the server can't find %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_DIAGNOSTIC_DNS_NOT_FOUND = 14,
  // board info is empty
  errorpb_ErrorTypeOptions_ERROR_TYPE_BOARD_INFO_EMPTY = 15,
  // the file extension %s is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_FILE_EXTENSION_INVALID = 100,
  // the file url scheme %s is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_FILE_URL_SCHEME_INVALID = 101,
  // the file %s is the same as the local configuration
  errorpb_ErrorTypeOptions_ERROR_TYPE_CONFIG_FILE_SAME_AS_LOCAL_INVALID = 102,
  // the priorirty %s  should be a multiple of 4096
  errorpb_ErrorTypeOptions_ERROR_TYPE_STP_PRIORITY_INVALID = 110,
  // the interface port %s does not allowed to be updated
  errorpb_ErrorTypeOptions_ERROR_TYPE_INTERFACE_PORT_INVALID = 111,
  // the interface vlan %s does not allowed to be updated
  errorpb_ErrorTypeOptions_ERROR_TYPE_INTERFACE_VLAN_INVALID = 112,
  // the interface trunk %s does not allowed to be updated
  errorpb_ErrorTypeOptions_ERROR_TYPE_INTERFACE_TRUNK_INVALID = 113,
  // the interface %s Q-in-Q ether type does not allowed to set %s when the mode is %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_Q_IN_Q_ETHER_TYPE_INVALID = 114,
  // the firmware file is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_MAINTENANCE_FIRMWARE_INVALID = 120,
  // the crt file url scheme %s is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_CRT_FILE_URL_SCHEME_INVALID = 121,
  // the key file url scheme %s is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_KEY_FILE_URL_SCHEME_INVALID = 122,
  // the ca file url scheme %s is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_CA_FILE_URL_SCHEME_INVALID = 123,
  // the crt content is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_CRT_CONTENT_INVALID = 124,
  // the ca content is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_CA_CONTENT_INVALID = 125,
  // the time zone %s is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_TIME_ZONE_INVALID = 126,
  // the time datetime %s is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_TIME_DATE_TIME_INVALID = 127,
  // the verison is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_POE_FIRMWARE_VERSION_INVALID = 128,
  // the custom_remote ID can't be empty when the remote ID source is user defined
  errorpb_ErrorTypeOptions_ERROR_TYPE_DHCP_CUSTOM_REMOTE_ID_INVALID = 129,
  // the basic ip range of the DHCP Server is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_DHCP_SERVER_BASIC_IP_INVALID = 130,
  // the basic lease time of the DHCP Server is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_DHCP_SERVER_BASIC_LEASE_TIME_INVALID = 131,
  // the ip of the macBased entry is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_DHCP_SERVER_MAC_BASED_IP_INVALID = 132,
  // the macAddress of the macBased entry is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_DHCP_SERVER_MAC_BASED_MACADDRESS_INVALID = 133,
  // the ip of the portBased entry is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_DHCP_SERVER_PORT_BASED_IP_INVALID = 134,
  // the portNo of the portBased entry is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_DHCP_SERVER_PORT_BASED_PORT_NO_INVALID = 135,
  // the file %s write failed
  errorpb_ErrorTypeOptions_ERROR_TYPE_FILE_WRITE_FAILED = 201,
  // import config failed
  errorpb_ErrorTypeOptions_ERROR_TYPE_CONFIG_IMPORT_FAILED = 202,
  // export config failed
  errorpb_ErrorTypeOptions_ERROR_TYPE_CONFIG_EXPORT_FAILED = 203,
  // the file %s read failed
  errorpb_ErrorTypeOptions_ERROR_TYPE_CONFIG_FILE_READ_FAILED = 204,
  // the file %s unmarshals failed
  errorpb_ErrorTypeOptions_ERROR_TYPE_CONFIG_FILE_UNMARSHAL_FALIED = 205,
  // the file %s validates failed, because %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_CONFIG_FILE_VALIDATE_FAILED = 206,
  // the file %s write failed
  errorpb_ErrorTypeOptions_ERROR_TYPE_CONFIG_FILE_WRITE_FAILED = 207,
  // the file %s marshal failed
  errorpb_ErrorTypeOptions_ERROR_TYPE_CONFIG_MARSHAL_FALIED = 208,
  // the file run script failed, because %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_CLI_ACTION_FAILED = 210,
  // authentication failed  , because %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACCESS_AUTHENTICATION_FAILED = 211,
  // to execute trace route failed
  errorpb_ErrorTypeOptions_ERROR_TYPE_DIAGNOSTIC_TRACE_ROUTE_FAILED = 212,
  // execute the snapshot fail because %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_MAINTENANCE_EXECUTE_FAILED = 214,
  // the file %s is uploading fail
  errorpb_ErrorTypeOptions_ERROR_TYPE_MAINTENANCE_UPLOAD_FAILED = 215,
  // the file %s is downloading fail
  errorpb_ErrorTypeOptions_ERROR_TYPE_MAINTENANCE_DOWNLOAD_FAILED = 216,
  // the file %s is upgrading fail
  errorpb_ErrorTypeOptions_ERROR_TYPE_MAINTENANCE_UPGRADE_FAILED = 217,
  // the crt file %s is downloading fail
  errorpb_ErrorTypeOptions_ERROR_TYPE_CRT_DOWNLOAD_FAILED = 218,
  // the key file %s is downloading fail
  errorpb_ErrorTypeOptions_ERROR_TYPE_KEY_DOWNLOAD_FAILED = 219,
  // the ca file %s is downloading fail
  errorpb_ErrorTypeOptions_ERROR_TYPE_CA_DOWNLOAD_FAILED = 220,
  // the crt file %s is uploading fail
  errorpb_ErrorTypeOptions_ERROR_TYPE_CRT_UPLOAD_FAILED = 221,
  // the key file %s is uploading fail
  errorpb_ErrorTypeOptions_ERROR_TYPE_KEY_UPLOAD_FAILED = 222,
  // the ca file %s is uploading fail
  errorpb_ErrorTypeOptions_ERROR_TYPE_CA_UPLOAD_FAILED = 223,
  // the format %s is invalid
  errorpb_ErrorTypeOptions_ERROR_TYPE_TIME_PARSE_FAILED = 224,
  // I2C read failed
  errorpb_ErrorTypeOptions_ERROR_TYPE_I2C_READ_FAILED = 225,
  // I2C write failed
  errorpb_ErrorTypeOptions_ERROR_TYPE_I2C_WRITE_FAILED = 226,
  // PoE command failed
  errorpb_ErrorTypeOptions_ERROR_TYPE_POE_COMMAND_FAILED = 227,
  // the id %s must be inside range [%s, %s]
  errorpb_ErrorTypeOptions_ERROR_TYPE_ID_OUT_OF_RANGE = 300,
  // the group id %s must be inside range [%s, %s]
  errorpb_ErrorTypeOptions_ERROR_TYPE_GROUP_ID_OUT_OF_RANGE = 301,
  // The quantity of entries has reached the limit of maximum entries [%s]
  errorpb_ErrorTypeOptions_ERROR_TYPE_ENTRY_OUT_OF_RANGE = 302,
  // max age need to meet formulas: MaxAge >= (2x(HelloTime+1))
  errorpb_ErrorTypeOptions_ERROR_TYPE_STP_HELLO_TIME_OUT_OF_RANGE = 310,
  // max age need to meet formulas: (2x(ForwardDelay-1)) >= MaxAge >= (2x(HelloTime+1))
  errorpb_ErrorTypeOptions_ERROR_TYPE_STP_MAX_AGE_OUT_OF_RANGE = 311,
  // 	max age need to meet formulas: (2x(ForwardDelay-1)) >= MaxAge
  errorpb_ErrorTypeOptions_ERROR_TYPE_STP_FORWARD_DELAY_OUT_OF_RANGE = 312,
  // the queueNo %s must be inside range [%s, %s]
  errorpb_ErrorTypeOptions_ERROR_TYPE_QOS_QUEUE_NO_OUT_OF_RANGE = 313,
  // the priority %s must be inside range [%s, %s]
  errorpb_ErrorTypeOptions_ERROR_TYPE_QOS_PRIORITY_OUT_OF_RANGE = 314,
  // the cosNo %s must be inside range [%s, %s]
  errorpb_ErrorTypeOptions_ERROR_TYPE_QOS_COS_NO_OUT_OF_RANGE = 315,
  // the dscpNo %s must be inside range [%s, %s]
  errorpb_ErrorTypeOptions_ERROR_TYPE_QOS_DSCP_NO_OUT_OF_RANGE = 316,
  // the %s must contain between %s and %s items, inclusive
  errorpb_ErrorTypeOptions_ERROR_TYPE_AU_DROP_LIST_OUT_OF_RANGE = 320,
  // the %s must contain between %s and %s items, inclusive
  errorpb_ErrorTypeOptions_ERROR_TYPE_AU_FORWARD_LIST_OUT_OF_RANGE = 321,
  // the %s must contain between %s and %s items, inclusive
  errorpb_ErrorTypeOptions_ERROR_TYPE_AU_LIMIT_OUT_OF_RANGE = 322,
  // the %s must contain between %s and %s items, inclusive
  errorpb_ErrorTypeOptions_ERROR_TYPE_AU_SECUTIRY_LEARNING_OUT_OF_RANGE = 323,
  // the %s must contain between %s and %s items, inclusive
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_GROUP_ID_OUT_OF_RANGE = 324,
  // the %s must contain between %s and %s items, inclusive
  errorpb_ErrorTypeOptions_ERROR_TYPE_LACP_GROUP_MEMBER_OUT_OF_RANGE = 325,
  // the %s must contain between %s and %s items, inclusive
  errorpb_ErrorTypeOptions_ERROR_TYPE_LACP_PRIORITY_OUT_OF_RANGE = 326,
  // the %s must contain between %s and %s items, inclusive
  errorpb_ErrorTypeOptions_ERROR_TYPE_LACP_ID_OUT_OF_RANGE = 327,
  // The quantity of sessions has reached the limit of maximum sessions [%s]
  errorpb_ErrorTypeOptions_ERROR_TYPE_MIRRORING_SESSION_OUT_OF_RANGE = 330,
  // the value %s must be inside range [%s, %s]
  errorpb_ErrorTypeOptions_ERROR_TYPE_HW_VALUE_OUT_OF_RANGE = 331,
  // the value %s must be inside range [%s, %s]
  errorpb_ErrorTypeOptions_ERROR_TYPE_POE_KEY_OUT_OF_RANGE = 332,
  // the value %s must be inside range [%s, %s]
  errorpb_ErrorTypeOptions_ERROR_TYPE_POE_BUDGET_OUT_OF_RANGE = 333,
  // the value %s must be inside range [%s, %s]
  errorpb_ErrorTypeOptions_ERROR_TYPE_MONITOR_SCORLL_VALUE_OUT_OF_RANGE = 334,
  // the name %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_NAME_NOT_EXIST = 400,
  // the id %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_ID_NOT_EXIST = 401,
  // the index %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_INDEX_NOT_EXIST = 402,
  // the group id %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_GROUP_ID_NOT_EXIST = 404,
  // the mac addres %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_MAC_ADDRESS_NOT_EXIST = 405,
  // the entry %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_ENTRY_NOT_EXIST = 406,
  // the interface does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_INTERFACE_NOT_EXIST = 410,
  // the interface port %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_INTERFACE_PORT_NOT_EXIST = 411,
  // the interface vlan %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_INTERFACE_VLAN_NOT_EXIST = 412,
  // the interface trunk %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_INTERFACE_TRUNK_NOT_EXIST = 413,
  // the queueNo %s does not exist in queue list
  errorpb_ErrorTypeOptions_ERROR_TYPE_QOS_QUEUE_NO_NOT_EXIST = 420,
  // the authentication server name %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACCESS_SERVER_NAME_NOT_EXIST = 421,
  // the group name %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACCESS_GROUP_NAME_NOT_EXIST = 422,
  // the restriction name %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACCESS_RESTRICTION_NAME_NOT_EXIST = 423,
  // the user name %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACCESS_USER_NAME_NOT_EXIST = 424,
  // the binding name %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACL_BINDING_NAME_NOT_EXIST = 430,
  // the flow name %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACL_FLOW_NAME_NOT_EXIST = 431,
  // the rule name %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACL_RULE_NAME_NOT_EXIST = 432,
  // the name %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACL_NAME_NOT_EXIST = 433,
  // the ingress rule name %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACL_INGRESS_RULE_NAME_NOT_EXIST = 434,
  // the egress rule name %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACL_EGRESS_RULE_NAME_NOT_EXIST = 435,
  // the filter id %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_FILTER_ID_NOT_EXIST = 440,
  // the group id %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_GROUP_ID_NOT_EXIST = 441,
  // the management vlan id %s does not exist in filter list(the vlan filter should be enabled)
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_MANAGEMENT_VLAN_ID_NOT_EXIST = 442,
  // the unauthorized vlan id %s does not exist in filter list(the vlan filter should be enabled)
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_UNAUTHORIZED_VLAN_ID_NOT_EXIST = 443,
  // the interface %s default vlan id %s does not exist in filter list(the vlan filter should be enabled)
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_DEFAULT_VLAN_ID_NOT_EXIST = 444,
  // the interface %s tagged vlan id %s does not exist in filter list(the vlan filter should be enabled)
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_TAGGED_VLAN_ID_NOT_EXIST = 445,
  // the interface %s untagged vlan id %s does not exist in filter list(the vlan filter should be enabled)
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_UNTAGGED_VLAN_ID_NOT_EXIST = 446,
  // the source vlan id %s does not exist in filter list
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_SOURCE_VLAN_ID_NOT_EXIST = 447,
  // the translated vlan id %s does not exist in filter list
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_TRANSLATED_VLAN_ID_NOT_EXIST = 448,
  // the certificate id %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_FILES_WEB_CERTIFICATE_ID_NOT_EXIST = 450,
  // the certificate id %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_FILES_SNMP_CERTIFICATE_ID_NOT_EXIST = 451,
  // the certificate id %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_FILES_SNMP_MANAGER_CERTIFICATE_ID_NOT_EXIST = 452,
  // the user %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_FILES_SNMP_MANAGER_USER_NOT_EXIST = 453,
  // the drop entry %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_AU_DROP_ENTRY_NOT_EXIST = 460,
  // the forward entry %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_AU_FORWARD_ENTRY_NOT_EXIST = 461,
  // the multicast entry %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_AU_MULTICAST_ENTRY_NOT_EXIST = 462,
  // the multicast igmp vlan id %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_MULTICAST_IGMP_VLAN_ID_NOT_EXIST = 465,
  // the multicast mld vlan id %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_MULTICAST_MLD_VLAN_ID_NOT_EXIST = 466,
  // the multicast unregistered vlan id %s does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_MULTICAST_UNREGISTERED_VLAN_ID_NOT_EXIST = 467,
  // the verison does not exist
  errorpb_ErrorTypeOptions_ERROR_TYPE_POE_FIRMWARE_VERSION_NOT_EXIST = 468,
  // the name %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_NAME_EXIST = 500,
  // the id %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_ID_EXIST = 501,
  // the group id %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_GROUP_ID_EXIST = 502,
  // the mac addres %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_MAC_ADDRESS_EXIST = 503,
  // the index %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_INDEX_EXIST = 504,
  // the entry exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_ENTRY_EXIST = 505,
  // the interface port %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_INTERFACE_PORT_EXIST = 510,
  // the interface vlan %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_INTERFACE_VLAN_EXIST = 511,
  // the interface trunk %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_INTERFACE_TRUNK_EXIST = 512,
  // the authentication server name %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACCESS_SERVER_NAME_EXIST = 520,
  // the group name %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACCESS_GROUP_NAME_EXIST = 521,
  // the restriction name %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACCESS_RESTRICTION_NAME_EXIST = 522,
  // the user name %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACCESS_USER_NAME_EXIST = 523,
  // the entry %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_AU_DROP_ENTRY_EXIST = 524,
  // the forwarad entry %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_AU_FORWARD_ENTRY_EXIST = 525,
  // the binding name %s exits
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACL_BINDING_NAME_EXIST = 526,
  // the flow name %s exits
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACL_FLOW_NAME_EXIST = 527,
  // the rule name %s exits
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACL_RULE_NAME_EXIST = 528,
  // the name %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACL_NAME_EXIST = 529,
  // the filter id %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_FILTER_ID_EXIST = 530,
  // the group id %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_GROUP_ID_EXIST = 531,
  // the source vlan id %s exists in translated list
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_SOURCE_VLAN_ID_EXIST = 532,
  // the user %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_FILES_SNMP_MANAGER_USER_EXIST = 533,
  // the multicast entry %s exists
  errorpb_ErrorTypeOptions_ERROR_TYPE_MULTICAST_STATIC_ENTRY_EXIST = 550,
  // the rule priorty %s exits
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACL_RULE_PRIORITY_EXIST = 551,
  // the feature is not supported
  errorpb_ErrorTypeOptions_ERROR_TYPE_FEATURE_NOT_SUPPORT = 600,
  // the file url scheme %s does not support
  errorpb_ErrorTypeOptions_ERROR_TYPE_URL_SCHEME_NOT_SUPPORT = 601,
  // the board name %s is not supported
  errorpb_ErrorTypeOptions_ERROR_TYPE_DEVICE_BOARD_NOT_SUPPORT = 602,
  // the interface port %s is not supported
  errorpb_ErrorTypeOptions_ERROR_TYPE_INTERFACE_PORT_NOT_SUPPORT = 603,
  // the interface vlan %s is not supported
  errorpb_ErrorTypeOptions_ERROR_TYPE_INTERFACE_VLAN_NOT_SUPPORT = 604,
  // the interface trunk %s is not supported
  errorpb_ErrorTypeOptions_ERROR_TYPE_INTERFACE_TRUNK_NOT_SUPPORT = 605,
  // the time zone is not support
  errorpb_ErrorTypeOptions_ERROR_TYPE_TIME_ZONE_NOT_SUPPORT = 606,
  // the name %s is occupied.
  errorpb_ErrorTypeOptions_ERROR_TYPE_NAME_OCCUPIED = 650,
  // the group id %s is occupied
  errorpb_ErrorTypeOptions_ERROR_TYPE_GROUP_ID_OCCUPIED = 651,
  // the vlan id %s is occupied
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_ID_OCCUPIED = 652,
  // the interface port %s is used by other trunk
  errorpb_ErrorTypeOptions_ERROR_TYPE_LACP_GROUP_MEMBER_OCCUPIED = 653,
  // the RSPAN is occupied
  errorpb_ErrorTypeOptions_ERROR_TYPE_RSPAN_OCCUPIED = 654,
  // the Interface %s is occupied
  errorpb_ErrorTypeOptions_ERROR_TYPE_INTERFACE_OCCUPIED = 655,
  // the default vlan id %s can not be changed
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_DEFAULT_VLAN_CAN_NOT_BE_CHANGED = 900,
  // the interface %s tagged list should include default vlan id %s when the mode is %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_TAGGED_LIST_INCLUDE_DEFAULT_VLAN = 901,
  // the interface %s tagged list should not include default vlan id %s when the mode is %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_TAGGED_LIST_EXCLUDE_DEFAULT_VLAN = 902,
  // the interface %s untagged list should include default vlan id %s when the mode is %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_UNTAGGED_LIST_INCLUDE_DEFAULT_VLAN = 903,
  // the interface %s untagged list should not include default vlan id %s when the mode is %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_UNTAGGED_LIST_EXCLUDE_DEFAULT_VLAN = 904,
  // the interface %s hold timer interval %s should less than half of join timer interval %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_GVRP_HOLD_TIMER_SHOULD_LESS_THAN_HALF_OF_JOIN_TIMER = 905,
  // the interface %s join timer interval %s should less than half of leave timer interval %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_GVRP_JOIN_TIMER_SHOULD_LESS_THAN_HALF_OF_LEAVE_TIMER = 906,
  // the interface %s leave timer interval %s should less than leave all timer interval %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_VLAN_GVRP_LEAVE_TIMER_SHOULD_LESS_THAN_LEAVE_ALL_TIMER = 907,
  // can't enable DHCPServer while DHCPRelay or DHCPClient is enabled
  errorpb_ErrorTypeOptions_ERROR_TYPE_DHCP_CAN_NOT_ENABLE_SERVER = 908,
  // can't enable DHCPRelay while DHCPServer or DHCPClient is enabled
  errorpb_ErrorTypeOptions_ERROR_TYPE_DHCP_CAN_NOT_ENABLE_RELAY = 909,
  // can't do this action because dhcp server is disabled
  errorpb_ErrorTypeOptions_ERROR_TYPE_DHCP_SERVER_SERVER_DISABLED = 910,
  // the start IP and end IP should not be equal
  errorpb_ErrorTypeOptions_ERROR_TYPE_DHCP_SERVER_IP_RANGE_EQUAL = 911,
  // the start IP should be smaller than end IP
  errorpb_ErrorTypeOptions_ERROR_TYPE_DHCP_SERVER_START_IP_SHOULD_BE_SMALLER_THAN_END_IP = 912,
  // the switch system is upgrading, please try again later
  errorpb_ErrorTypeOptions_ERROR_TYPE_MAINTENANCE_UPGRADING = 913,
  // IPv6 management cannot be disabled when MLD snooping is enabled
  errorpb_ErrorTypeOptions_ERROR_TYPE_NETWORK_MLD_ENABLED = 914,
  // IPv6 ICMP auto address cannot be enabled when IPv6 management is disabled
  errorpb_ErrorTypeOptions_ERROR_TYPE_NETWORK_ICMP_NOT_ENABLED = 915,
  // IPv6 auto configuration cannot be enabled when IPv6 management is disabled
  errorpb_ErrorTypeOptions_ERROR_TYPE_NETWORK_AUTO_NOT_ENABLED = 916,
  // IPv6 static IP address cannot be configuared when IPv6 management is disabled
  errorpb_ErrorTypeOptions_ERROR_TYPE_NETWORK_STATIC_IP_NOT_CONFIGURED = 917,
  // IPv6 DNS server cannot be configuared when IPv6 management is disabled
  errorpb_ErrorTypeOptions_ERROR_TYPE_NETWORK_DNS_SERVER_NOT_CONFIGURED = 918,
  // encrypted password creation error
  errorpb_ErrorTypeOptions_ERROR_TYPE_ACCESS_ENCRYPTED_PASSWORD_CREATED_FAILED = 919,
  // feil unexpected error
  errorpb_ErrorTypeOptions_ERROR_TYPE_FILE_UNEXPECTED_ERROR = 920,
  // NTP not enable
  errorpb_ErrorTypeOptions_ERROR_TYPE_NTP_NOT_ENABLE = 921,
  // OS command execute error
  errorpb_ErrorTypeOptions_ERROR_TYPE_COMMAND_EXECUTE_FAILED = 922,
  // MSTP is updating, please wait
  errorpb_ErrorTypeOptions_ERROR_TYPE_MSTP_IS_UPDATEING = 923,
  // Mirroring Err : %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_MIRRORING_ERROR = 924,
  // Port Security Err : %s
  errorpb_ErrorTypeOptions_ERROR_TYPE_PORT_SECURITY_ERROR = 925,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct errorpb_Detail {
  uint64_t Code;
};

struct errorpb_PreparingDetail {
  enum errorpb_ErrorTypeOptions ErrorOption;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_error_error
