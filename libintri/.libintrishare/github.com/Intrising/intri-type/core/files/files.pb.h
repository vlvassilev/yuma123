
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_files_files
#define _H_intri_pb_github_com_Intrising_intri_type_core_files_files

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

enum filespb_CertificateTypeOptions {
  // [Web]
  filespb_CertificateTypeOptions_CERTIFICATE_TYPE_WEB = 0,
  // [SNMP]
  filespb_CertificateTypeOptions_CERTIFICATE_TYPE_SNMP = 1,
};

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct filespb_Config {
  bool FTPEnabled;
  struct filespb_ActivateCertificate *Certificate;
};

// CertificateID defined in CertificateIDList
// CertificateIDList is derived from GetCertificateList()
struct filespb_ActivateCertificate {
  struct filespb_CertificateID *Web;
  struct filespb_CertificateID *SnmpAgent;
  // CertificateID supplicant = 3;
  struct filespb_CertificateUserIDList *SnmpManager;
};

// Certificate 
struct filespb_CertificateID {
  char *ID;
};

struct filespb_CertificateIDList {
  // boundary is according to device.GetBoundary().Files.CertificatedRange
  unsigned int List_Len; // auto-gen: for list
  char **List;
};

struct filespb_CertificateData {
  // Index (for import / remove); unique. ID can't duplicated among CertificateIDList when import.
  char *ID;
  char *CrtPath;
  char *KeyPath;
  bool FTPSEnabled;
  char *AuthCrtPath;
};

struct filespb_CertificateUserID {
  // Index (for set); unique.Only ID is defined among CertificateIDList.
  char *ID;
  char *Username;
};

struct filespb_CertificateUserIDList {
  // boundary is according to device.GetBoundary().Access.Users
  unsigned int List_Len; // auto-gen: for list
  struct filespb_CertificateUserID **List;
};

// internal
struct filespb_CertificateType {
  enum filespb_CertificateTypeOptions Type;
};

struct filespb_CertificateInfo {
  char *CrtPath;
  char *KeyPath;
  char *Passphrase;
  char *AuthCrtPath;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_files_files
