
#ifndef _H_intri_pb_github_com_Intrising_intri_type_core_maintenance_maintenance
#define _H_intri_pb_github_com_Intrising_intri_type_core_maintenance_maintenance

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Import To C Include                                                              *
 *                                                                                                    *
 **************************************************************************************************** */
#include "../../../../../github.com/Intrising/intri-type/common/common.pb.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/empty.pb.h"
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

/* ****************************************************************************************************
 *                                                                                                    *
 * Auto-Gen ProtoBuf Messages To C Structs                                                            *
 *                                                                                                    *
 **************************************************************************************************** */

struct maintenancepb_FirmwareInfo {
  char *Version;
  char *BuildDate;
  char *BuildNumber;
};

struct maintenancepb_UpgradePathRequest {
  // The full path with the file name on the FTP server
// ex: ftp://192.168.1.1/firmware/dragonV10015.upg
// ex: ftps://192.168.1.1/config/setting.yml
// [NOTE]:
// 1. the path cannot contain a certain character
// ex: ";"
// 2. the regex of the path doesn't check the extension, core will check it.  https://github.com/Intrising/intri-type/issues/282
// 3. Example of the ftps/ftp/sftp
// FTP:
//     path: ftp://192.168.1.1/firmware/dragonV10015.upg
//     ftpsEnabled: false
// SFTP:
//     path: sftp://192.168.1.1/config/setting.yml
//     ftpsEnabled: false
// FTPS:
//     path: ftp://192.168.1.1/firmware/dragonV10015.upg
//     ftpsEnabled: true
  char *Path;
  bool FTPSEnabled;
  bool RebootAfterAction;
};
#endif // _H_intri_pb_github_com_Intrising_intri_type_core_maintenance_maintenance
