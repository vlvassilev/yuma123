
#include "../.libintrishare/libintrishare.h"

void testSetOneofToGo()
{
  struct emptypb_Empty *epty = malloc(sizeof(*(epty)));
  struct aclpb_ACEEntry *in = malloc(sizeof(*(in)));
  in->Action = aclpb_RuleActionTypeOptions_RULE_ACTION_TYPE_PERMIT;
  in->Name = "ian-test";
  in->Priority = 10;
  in->ParamType = aclpb_RuleParamTypeOptions_RULE_PARAM_TYPE_MAC;
  in->TimeRangeName = "";

  // required
  in->Param_Union_Option = aclpb_ACEEntry_Param_Union_Options_Mac;
  in->Param.Param_Mac = malloc(sizeof(*(in->Param.Param_Mac)));
  in->Param.Param_Mac->Source = malloc(sizeof(*(in->Param.Param_Mac->Source)));
  in->Param.Param_Mac->Source->Address = "aa:bb:cc:dd:ee:ff";

  acl_ACL_AddACE(in, epty);

  free(epty);
  free(in);
}

void testGetOneofToGo()
{
  struct emptypb_Empty *epty = malloc(sizeof(*(epty)));
  struct aclpb_ACEList *out = malloc(sizeof(*(out)));
  acl_ACL_GetACEList(epty, out);

  for (int i = 0; i < out->List_Len; i++)
  {
    printf("\n ===");
    printf("\n Name = %s", out->List[i]->Name);
    printf("\n Action = %d", out->List[i]->Action);
    printf("\n Param_Union_Option = %d", out->List[i]->Param_Union_Option);
    switch (out->List[i]->Param_Union_Option)
    {
      // case aclpb_ACEEntry_Param_Union_Options_MacIPv4:
      //   break;
      // case aclpb_ACEEntry_Param_Union_Options_MacIPv6:
      //   break;
    case aclpb_ACEEntry_Param_Union_Options_IPv4:
      printf("\n IPv4->Source->Address = %s", out->List[i]->Param.Param_IPv4->Source->Address);
      printf("\n IPv4->Source->AddressMask = %s", out->List[i]->Param.Param_IPv4->Source->AddressMask);
      printf("\n IPv4->Destination->Address = %s", out->List[i]->Param.Param_IPv4->Destination->Address);
      printf("\n IPv4->Destination->AddressMask = %s", out->List[i]->Param.Param_IPv4->Destination->AddressMask);
      printf("\n IPv4->Layer4Port->Source->PortNumber = %d", out->List[i]->Param.Param_IPv4->Layer4Port->Source->PortNumber);
      printf("\n IPv4->Layer4Port->Source->PortNumberMask = %d", out->List[i]->Param.Param_IPv4->Layer4Port->Source->PortNumberMask);
      printf("\n IPv4->Layer4Port->Destination->PortNumber = %d", out->List[i]->Param.Param_IPv4->Layer4Port->Destination->PortNumber);
      printf("\n IPv4->Layer4Port->Destination->PortNumberMask = %d", out->List[i]->Param.Param_IPv4->Layer4Port->Destination->PortNumberMask);
      printf("\n IPv4->Protocol->Protocol = %s", out->List[i]->Param.Param_IPv4->Protocol->Protocol);
      printf("\n IPv4->Protocol->ProtocolMask = %s", out->List[i]->Param.Param_IPv4->Protocol->ProtocolMask);
      break;
    case aclpb_ACEEntry_Param_Union_Options_IPv6:
      printf("\n IPv6->Source->Address = %s", out->List[i]->Param.Param_IPv6->Source->Address);
      printf("\n IPv6->Source->AddressMask = %s", out->List[i]->Param.Param_IPv6->Source->AddressMask);
      printf("\n IPv6->Destination->Address = %s", out->List[i]->Param.Param_IPv6->Destination->Address);
      printf("\n IPv6->Destination->AddressMask = %s", out->List[i]->Param.Param_IPv6->Destination->AddressMask);
      printf("\n IPv6->Layer4Port->Source->PortNumber = %d", out->List[i]->Param.Param_IPv6->Layer4Port->Source->PortNumber);
      printf("\n IPv6->Layer4Port->Source->PortNumberMask = %d", out->List[i]->Param.Param_IPv6->Layer4Port->Source->PortNumberMask);
      printf("\n IPv6->Layer4Port->Destination->PortNumber = %d", out->List[i]->Param.Param_IPv6->Layer4Port->Destination->PortNumber);
      printf("\n IPv6->Layer4Port->Destination->PortNumberMask = %d", out->List[i]->Param.Param_IPv6->Layer4Port->Destination->PortNumberMask);
      printf("\n IPv6->NextHeader->Protocol = %s", out->List[i]->Param.Param_IPv6->NextHeader->Protocol);
      printf("\n IPv6->NextHeader->ProtocolMask = %s", out->List[i]->Param.Param_IPv6->NextHeader->ProtocolMask);
      break;
    case aclpb_ACEEntry_Param_Union_Options_Mac:
      printf("\n Mac->Source->Address = %s", out->List[i]->Param.Param_Mac->Source->Address);
      printf("\n Mac->Source->AddressMask = %s", out->List[i]->Param.Param_Mac->Source->AddressMask);
      printf("\n Mac->Destination->Address = %s", out->List[i]->Param.Param_Mac->Destination->Address);
      printf("\n Mac->Destination->AddressMask = %s", out->List[i]->Param.Param_Mac->Destination->AddressMask);
      printf("\n Mac->EtherType->EtherTypeMask = %s", out->List[i]->Param.Param_Mac->EtherType->EtherTypeMask);
      printf("\n Mac->EtherType->Type = %s", out->List[i]->Param.Param_Mac->EtherType->Type);
      printf("\n Mac->VlanId = %d", out->List[i]->Param.Param_Mac->VlanId);
      break;
    };
    printf("\n");
  };

  free(epty);
  free(out);
}

int main()
{
  testGetOneofToGo();
  return 0;
}