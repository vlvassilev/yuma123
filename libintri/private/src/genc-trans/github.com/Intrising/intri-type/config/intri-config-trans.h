// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_intri_config_trans
#define _H_intri_config_trans

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

#include "../../../../../../../.libintrishare/libintrishare.h"

extern status_t build_to_xml_config_ImportAction (
    val_value_t *parentval,
    struct configpb_ImportAction *entry);

extern status_t build_to_xml_config_ExportAction (
    val_value_t *parentval,
    struct configpb_ExportAction *entry);

extern status_t build_to_xml_config_SaveModeStatus (
    val_value_t *parentval,
    struct configpb_SaveModeStatus *entry);

extern status_t build_to_xml_config_RestoreDefaultType (
    val_value_t *parentval,
    struct configpb_RestoreDefaultType *entry);

extern status_t build_to_xml_config_AllServicesConfig (
    val_value_t *parentval,
    struct configpb_AllServicesConfig *entry);

extern status_t build_to_xml_config_ValidateConfigResult (
    val_value_t *parentval,
    struct configpb_ValidateConfigResult *entry);


#endif /* _H_intri_config_trans */

