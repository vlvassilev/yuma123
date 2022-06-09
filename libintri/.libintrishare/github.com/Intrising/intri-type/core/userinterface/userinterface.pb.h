
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_userinterface_userinterface
#define _H_intri_pb_github_com_Intrising_intri_type_core_userinterface_userinterface

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/common/common.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
#include <stdbool.h>

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Enums To C Enums                                                                 *
 *                                                                                                    *
 **************************************************************************************************** */

enum userinterfacepb_SNMPV3ConfigSecurityModelTypeOptions {
  // [USM] Assign the USM as the SNMPv3 security model
  userinterfacepb_SNMPV3ConfigSecurityModelTypeOptions_SNMP_V3_CONFIG_SECURITY_MODEL_USM = 0,
  // [VCAM] Assign the VACM as the SNMPv3 security model
  userinterfacepb_SNMPV3ConfigSecurityModelTypeOptions_SNMP_V3_CONFIG_SECURITY_MODEL_VACM = 1,
  // [TSM] Assign the TSM as the SNMPv3 security model
  userinterfacepb_SNMPV3ConfigSecurityModelTypeOptions_SNMP_V3_CONFIG_SECURITY_MODEL_TSM = 2,
};

enum userinterfacepb_WebProtocolTypeOptions {
  // [Disabled] Web interface is disabled
  userinterfacepb_WebProtocolTypeOptions_WEB_PROTOCOL_TYPE_DISABLED = 0,
  // [HTTP Only] Standard client interface
  userinterfacepb_WebProtocolTypeOptions_WEB_PROTOCOL_TYPE_HTTP_UNSECURE = 1,
  // [HTTPS Only] Secure client interface
  userinterfacepb_WebProtocolTypeOptions_WEB_PROTOCOL_TYPE_HTTPS_SECURE = 2,
  // [HTTP & HTTPS] Both Standard & Secure client interface
  userinterfacepb_WebProtocolTypeOptions_WEB_PROTOCOL_TYPE_HTTP_AND_HTTPS = 3,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct userinterfacepb_CLIConfig {
  bool TelnetEnabled;
  bool SSHEnabled;
  char *WelcomeMessage;
  // The unit of this field is second
  long int InactivityTimeout;
};

// SNMP Config 
struct userinterfacepb_SNMPConfig {
  bool SNMPV1Enabled;
  bool SNMPV2CEnabled;
  char *GetCommunity;
  char *SetCommunity;
  // This user name must be an existed user
  char *SNMPV1V2Username;
  bool PermitV1V2SetCommands;
  bool SNMPV3Enabled;
  enum userinterfacepb_SNMPV3ConfigSecurityModelTypeOptions SecurityModel;
  char *SNMPEngineID;
  char *TrapEngineID;
};

// Web Config 
struct userinterfacepb_WebConfig {
  // The enum to control which protocol to serve
// NOTE: web server needs to be restart to activate the protocol changes
  enum userinterfacepb_WebProtocolTypeOptions Protocol;
  // The time that web will auto logout if detects no action, the unit is second
  long int WebTimeout;
  // NOTE: web server needs to be restart to activate the protocol changes
  long int HTTPPort;
  // NOTE: web server needs to be restart to activate the protocol changes
  long int HTTPSPort;
  // Only encrypted passphrase can be set in this web config
// If you need to use plaintext password, you need to use Service/Access/RunEncryptPassword for transformation
  char *EncryptedCertPassphrase;
  // This value will be used to change the WEB index.html's document.title directly
// Will take affect directly after user refresh
  char *LoginMessage;
};

struct userinterfacepb_Config {
  struct userinterfacepb_CLIConfig *Cli;
  struct userinterfacepb_SNMPConfig *Snmp;
  struct userinterfacepb_WebConfig *Web;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_userinterface_userinterface
