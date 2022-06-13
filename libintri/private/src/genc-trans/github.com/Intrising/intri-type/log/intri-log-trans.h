// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_intri_log_trans
#define _H_intri_log_trans

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

extern status_t build_to_xml_log_Config (
    val_value_t *parentval,
    struct logpb_Config *entry);

extern status_t build_to_xml_log_BasicConfig (
    val_value_t *parentval,
    struct logpb_BasicConfig *entry);

extern status_t build_to_xml_log_TargetConfig (
    val_value_t *parentval,
    struct logpb_TargetConfig *entry);

extern status_t build_to_xml_log_TargetConfigEntry (
    val_value_t *parentval,
    struct logpb_TargetConfigEntry *entry);

extern status_t build_to_xml_log_ActionConfig (
    val_value_t *parentval,
    struct logpb_ActionConfig *entry);

extern status_t build_to_xml_log_ActionConfigEntry (
    val_value_t *parentval,
    struct logpb_ActionConfigEntry *entry);

extern status_t build_to_xml_log_Statistics (
    val_value_t *parentval,
    struct logpb_Statistics *entry);

extern status_t build_to_xml_log_LogFileEntry (
    val_value_t *parentval,
    struct logpb_LogFileEntry *entry);

extern status_t build_to_xml_log_LogFiles (
    val_value_t *parentval,
    struct logpb_LogFiles *entry);


#endif /* _H_intri_log_trans */
