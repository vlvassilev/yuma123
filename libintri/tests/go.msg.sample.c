
#include "../.libintrishare/libintrishare.h"

void testGetMsgToC() {
  struct systempb_Config in;
  struct emptypb_Empty out;
  in.SysName = "ian";
  system_System_SetConfig(&in, &out);
}

void testGetMsgToGo() {
  struct emptypb_Empty in;
  struct systempb_Config out;
  system_System_GetConfig(&in, &out);
  printf("SysContact     : %s\n", out.SysContact);
  printf("SysGroup       : %s\n", out.SysGroup);
  printf("SysLocation    : %s\n", out.SysLocation);
  printf("SysName        : %s\n", out.SysName);
}

int main() {
  testGetMsgToC();
  testGetMsgToGo();
  return 0;
};