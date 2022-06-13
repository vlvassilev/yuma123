
#include "../.libintrishare/libintrishare.h"

void testSetListToGo()
{
  struct emptypb_Empty *epty = malloc(sizeof(*(epty)));

  struct vlanpb_PortsConfig *in = malloc(sizeof(*(in)));
  in->List_Len = 30;
  in->List = malloc(in->List_Len * sizeof(*(in->List)));
  for (int i = 0; i < in->List_Len; i++)
  {
    struct vlanpb_PortEntry *updateEntry = malloc(sizeof(*(updateEntry)));
    updateEntry->IdentifyNo = malloc(sizeof(*(updateEntry->IdentifyNo)));
    updateEntry->IdentifyNo->Type = devicepb_InterfaceTypeOptions_INTERFACE_TYPE_PORT;
    updateEntry->IdentifyNo->DeviceID = 0;
    updateEntry->IdentifyNo->PortNo = i + 1;
    updateEntry->Mode = vlanpb_PortConfigVLANModeTypeOptions_PORT_CONFIG_VLAN_MODE_TYPE_ACCESS;
    updateEntry->DefaultVlanID = 1;
    updateEntry->UnauthorizedVlanID = 1;
    updateEntry->FallBackVlanID = 1;
    updateEntry->QinQEthertype = vlanpb_PortConfigQinQEtherTypeOptions_PORT_CONFIG_QINQ_ETHERTYPE_TYPE_0X_8100;
    updateEntry->AcceptableFrametype = vlanpb_AcceptFrameTypeOptions_ACCEPT_FRAME_TYPE_ALL;
    updateEntry->TaggedList_Len = 0;
    updateEntry->TaggedList = malloc(sizeof(*(updateEntry->TaggedList)));
    updateEntry->UntaggedList_Len = 0;
    updateEntry->UntaggedList = malloc(sizeof(*(updateEntry->UntaggedList)));
    in->List[i] = updateEntry;

    free(updateEntry);
  };

  vlan_VLAN_UpdatePortsConfig(in, epty);

  free(epty);
  free(in);
}

void testGetListFromGo()
{
  struct emptypb_Empty *epty = malloc(sizeof(*(epty)));
  struct vlanpb_PortsConfig *out = malloc(sizeof(*(out)));

  vlan_VLAN_GetPortsConfig(epty, out);

  for (int i = 0; i < out->List_Len; i++)
  {
    printf("\n ===");
    printf("\n Type: %d, DeviceID: %d, PortNo: %d, VlanID: %d, LagNo: %d",
           out->List[i]->IdentifyNo->Type,
           out->List[i]->IdentifyNo->DeviceID,
           out->List[i]->IdentifyNo->PortNo,
           out->List[i]->IdentifyNo->VlanID,
           out->List[i]->IdentifyNo->LAGNo);
    printf("\n Mode                                     = %d", out->List[i]->Mode);
    printf("\n DefaultVlanID                            = %d", out->List[i]->DefaultVlanID);
    printf("\n FallBackVlanID                           = %d", out->List[i]->FallBackVlanID);
    printf("\n UnauthorizedVlanID                       = %d", out->List[i]->UnauthorizedVlanID);
    printf("\n QinQEthertype                            = %d", out->List[i]->QinQEthertype);
    printf("\n AcceptableFrametype                      = %d", out->List[i]->AcceptableFrametype);

    printf("\n TaggedList: ");
    for (int j = 0; j < out->List[i]->TaggedList_Len; j++)
    {
      printf("%d, ", out->List[i]->TaggedList[j]);
    };

    printf("\n UntaggedList: ");
    for (int j = 0; j < out->List[i]->UntaggedList_Len; j++)
    {
      printf("%d, ", out->List[i]->UntaggedList[j]);
    };
    printf("\n");
  };

  free(epty);
  free(out);
}

int main()
{
  testSetListToGo();
  testGetListFromGo();
  return 0;
}