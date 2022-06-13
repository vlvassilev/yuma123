
#include "../.libintrishare/libintrishare.h"

void testSetMapToGo(){
    // there has no map input in intri-type
};

void testGetMapToGo()
{

  struct vlanpb_Used *in = malloc(sizeof(*(in)));
  struct vlanpb_StatusMapping *out = malloc(sizeof(*(out)));

  in->Used = vlanpb_StatusUsedTypeOptions_STATUS_USED_TYPE_CONFIG;

  vlan_VLAN_GetStatus(in, out);

  for (int i = 0; i < out->Mapping_Len; i++)
  {
    printf("\n ===");
    printf("\n Key = %d", out->Mapping[i]->Key);

    printf("\n VlanID = %d", out->Mapping[i]->Value->VlanID);

    printf("\n TaggedPortList: ");
    for (int j = 0; j < out->Mapping[i]->Value->TaggedList_Len; j++)
    {
      printf("\n Type: %d, DeviceID: %d, PortNo: %d, VlanID: %d, LagNo: %d",
             out->Mapping[i]->Value->TaggedList[j]->Type,
             out->Mapping[i]->Value->TaggedList[j]->DeviceID,
             out->Mapping[i]->Value->TaggedList[j]->PortNo,
             out->Mapping[i]->Value->TaggedList[j]->VlanID,
             out->Mapping[i]->Value->TaggedList[j]->LAGNo);
    };

    printf("\n UntaggedPortList: ");
    for (int j = 0; j < out->Mapping[i]->Value->UntaggedList_Len; j++)
    {
      printf("\n Type: %d, DeviceID: %d, PortNo: %d, VlanID: %d, LagNo: %d",
             out->Mapping[i]->Value->UntaggedList[j]->Type,
             out->Mapping[i]->Value->UntaggedList[j]->DeviceID,
             out->Mapping[i]->Value->UntaggedList[j]->PortNo,
             out->Mapping[i]->Value->UntaggedList[j]->VlanID,
             out->Mapping[i]->Value->UntaggedList[j]->LAGNo);
    };
    printf("\n");
  };

  free(in);
  free(out);
};

int main()
{
  testGetMapToGo();
  return 0;
};