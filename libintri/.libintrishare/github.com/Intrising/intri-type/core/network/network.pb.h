
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_network_network
#define _H_intri_pb_github_com_Intrising_intri_type_core_network_network

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/common/common.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>
#include <stdint.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum networkpb_IPv4DhcpModeTypeOptions {
  // [Static]
  networkpb_IPv4DhcpModeTypeOptions_IPV4_DHCP_MODE_TYPE_DISABLED = 0,
  // [Use DHCP]
  networkpb_IPv4DhcpModeTypeOptions_IPV4_DHCP_MODE_TYPE_USE_DHCP = 1,
  // [Use DHCP with Option 66/67]
  networkpb_IPv4DhcpModeTypeOptions_IPV4_DHCP_MODE_TYPE_DHCP_WITH_OPTION_66_67 = 2,
};

enum networkpb_IPV4ConfigDefaultAddressSelectionTypeOptions {
  // [Primary]
  networkpb_IPV4ConfigDefaultAddressSelectionTypeOptions_IPV4_CONFIG_DEFAULT_ADDRESS_SELECTION_TYPE_PRIMARY = 0,
  // [Secondary]
  networkpb_IPV4ConfigDefaultAddressSelectionTypeOptions_IPV4_CONFIG_DEFAULT_ADDRESS_SELECTION_TYPE_SECONDARY = 1,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct networkpb_Config {
  struct networkpb_BasicConfig *Basic;
  struct networkpb_IPConfig *IP;
};

struct networkpb_BasicConfig {
  char *HostName;
  char *DomainName;
  int32_t LocalMTU;
};

struct networkpb_IPConfig {
  struct networkpb_IPv4Config *V4;
  struct networkpb_IPv6Config *V6;
};

struct networkpb_IPv4Config {
  enum networkpb_IPv4DhcpModeTypeOptions DHCPMode;
  struct networkpb_IPv4Static *Static;
};

struct networkpb_IPv4Static {
  char *IPAddress;
  char *SubnetMask;
  char *Gateway;
  char *DNSServer;
  char *SecondaryDeviceIP;
  char *SecondarySubnetMask;
  enum networkpb_IPV4ConfigDefaultAddressSelectionTypeOptions DefaultAddressSelection;
};

struct networkpb_IPv6Config {
  bool Enabled;
  bool IcmpAutoAddressEnabled;
  bool AutoConfigurationEnabled;
  struct networkpb_IPv6Static *Static;
};

struct networkpb_IPv6Static {
  char *DNSServer;
  char *IPAddress;
};

struct networkpb_IPv4Status {
  char *DynamicDeviceIP;
  char *DynamicSubnetMask;
  char *DynamicGateway;
  char *DynamicDNSServer1;
  char *DynamicDNSServer2;
  char *DynamicDNSServer3;
  char *DynamicDNSServer4;
  char *OutgoingDeviceIP;
};

struct networkpb_IPv6StatusEntry {
  char *IPAddress;
  enum commonpb_IPv6ScopeTypeOptions Scope;
};

struct networkpb_IPv6Status {
  unsigned int List_Len; // auto-gen: for list
  struct networkpb_IPv6StatusEntry **List;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_network_network
