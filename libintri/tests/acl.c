
#include "../.libintrishare/libintrishare.h"

int main()
{

  struct emptypb_Empty *out = malloc(sizeof(*(out)));
  struct commonpb_Name *inDelete = malloc(sizeof(*(inDelete)));
  inDelete->Name = "ian-test";
  acl_ACL_DeleteACE(inDelete, out);

  struct aclpb_ACEEntry *inAddUpdate = malloc(sizeof(*(inAddUpdate)));
  inAddUpdate->Action = aclpb_RuleActionTypeOptions_RULE_ACTION_TYPE_PERMIT;
  inAddUpdate->Name = inDelete->Name;
  inAddUpdate->Priority = 10;
  inAddUpdate->ParamType = aclpb_RuleParamTypeOptions_RULE_PARAM_TYPE_MAC;
  inAddUpdate->TimeRangeName = "";
  struct aclpb_RuleMAC *inAddUpdateParam = malloc(sizeof(*(inAddUpdateParam)));
  inAddUpdateParam->Source = malloc(sizeof(*(inAddUpdateParam->Source)));
  inAddUpdateParam->Source->Address = "aa:bb:cc:dd:ee:ff";
  inAddUpdate->Param_Union_Option = aclpb_ACEEntry_Param_Union_Options_Mac;
  inAddUpdate->Param.Param_Mac = inAddUpdateParam;
  acl_ACL_AddACE(inAddUpdate, out);

  inAddUpdateParam->Source->Address = "aa:bb:cc:dd:ee:00";
  acl_ACL_UpdateACE(inAddUpdate, out);
  return 0;
}