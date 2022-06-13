// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_intri_time_trans
#define _H_intri_time_trans

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

#include "../../../../../../../../.libintrishare/libintrishare.h"

extern status_t build_to_xml_time_Config (
    val_value_t *parentval,
    struct timepb_Config *entry);

extern status_t build_to_xml_time_ListTimeZones (
    val_value_t *parentval,
    struct timepb_ListTimeZones *entry);

extern status_t build_to_xml_time_Status (
    val_value_t *parentval,
    struct timepb_Status *entry);

extern status_t build_to_xml_time_RequestWithTimestamp (
    val_value_t *parentval,
    struct timepb_RequestWithTimestamp *entry);

extern status_t build_to_xml_time_RequestWithInt64 (
    val_value_t *parentval,
    struct timepb_RequestWithInt64 *entry);

extern status_t build_to_xml_time_Response (
    val_value_t *parentval,
    struct timepb_Response *entry);


#endif /* _H_intri_time_trans */
