
#include "../.libintrishare/libintrishare.h"

int main()
{
  struct vlanpb_PortsConfig *in = malloc(sizeof(*(in)));
  struct emptypb_Empty *out = malloc(sizeof(*(out)));

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
  };
  vlan_VLAN_UpdatePortsConfig(in, out);
  return 0;
}