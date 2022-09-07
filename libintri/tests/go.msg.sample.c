
#include "../.libintrishare/libintrishare.h"

void testGetMsgToC()
{
  struct systempb_Config *in = malloc(sizeof(*(in)));
  struct emptypb_Empty *out = malloc(sizeof(*(out)));
  GoString *errstr = malloc(sizeof(*errstr));
  in->SysName = "ian";
  system_System_SetConfig(in, out, errstr);
  if (errstr->n > 0)
  {
    printf("%s\n", errstr->p);
  }
  free(in);
  free(out);
  free(errstr);
}

void testGetMsgToGo()
{
  struct emptypb_Empty *in = malloc(sizeof(*(in)));
  struct systempb_Config *out = malloc(sizeof(*(out)));
  GoString *errstr = malloc(sizeof(*errstr));
  system_System_GetConfig(in, out, errstr);
  if (errstr->n > 0)
  {
    printf("%s\n", errstr->p);
  }
  printf("SysContact     : %s\n", out->SysContact);
  printf("SysGroup       : %s\n", out->SysGroup);
  printf("SysLocation    : %s\n", out->SysLocation);
  printf("SysName        : %s\n", out->SysName);
  free(in);
  free(out);
  free(errstr);
}

int main()
{
  testGetMsgToC();
  testGetMsgToGo();
  return 0;
};