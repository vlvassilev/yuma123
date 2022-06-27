// Code generated by protoc-gen-yang(*.c) DO NOT EDIT.

/*****************************************************************************************************
 * Copyright (C) 2017-2022 by Intrising
 *  - ian0113@intrising.com.tw
 * 
 * Generated by protoc-gen-yang@gen-yang
 * 
 *****************************************************************************************************/

#include <libxml/xmlstring.h>
#include "agt.h"
#include "agt_cb.h"
#include "agt_rpc.h"
#include "agt_timer.h"
#include "agt_util.h"
#include "dlq.h"
#include "ncx.h"
#include "ncx_feature.h"
#include "ncxmod.h"
#include "ncxtypes.h"
#include "procdefs.h"
#include "rpc.h"
#include "ses.h"
#include "status.h"
#include "val.h"
#include "val_util.h"
#include "xml_util.h"

#include "intri-snmptrap-trans.h"
#include "../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../github.com/Intrising/intri-type/core/access/intri-access-trans.h"
#include "../../../../github.com/Intrising/intri-type/core/userinterface/intri-userinterface-trans.h"
#include "../../../../github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"
status_t build_to_xml_snmptrap_SNMPTrapCounter (
    val_value_t *parentval,
    struct snmptrappb_SNMPTrapCounter *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "ErrorCounts",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->ErrorCounts;
  /* ---------------------------------------------------------------------------------------------------- */
  childval =  agt_make_object(
      parentval->obj,
    "TrapCounts",
    &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* int32 */
  VAL_INT(childval) = entry->TrapCounts;
  return res;
}

