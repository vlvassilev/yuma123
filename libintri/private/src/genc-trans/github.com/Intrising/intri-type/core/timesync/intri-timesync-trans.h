// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_intri_timesync_trans
#define _H_intri_timesync_trans

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

extern status_t build_to_xml_timesync_Config (
    val_value_t *parentval,
    struct timesyncpb_Config *entry);

extern status_t build_to_xml_timesync_GNSSConfig (
    val_value_t *parentval,
    struct timesyncpb_GNSSConfig *entry);

extern status_t build_to_xml_timesync_SyncEConfig (
    val_value_t *parentval,
    struct timesyncpb_SyncEConfig *entry);

extern status_t build_to_xml_timesync_SyncSourceConfig (
    val_value_t *parentval,
    struct timesyncpb_SyncSourceConfig *entry);

extern status_t build_to_xml_timesync_ReferenceOutput (
    val_value_t *parentval,
    struct timesyncpb_ReferenceOutput *entry);

extern status_t build_to_xml_timesync_ToDConfig (
    val_value_t *parentval,
    struct timesyncpb_ToDConfig *entry);

extern status_t build_to_xml_timesync_GNSStatus (
    val_value_t *parentval,
    struct timesyncpb_GNSStatus *entry);

extern status_t build_to_xml_timesync_SyncEStatus (
    val_value_t *parentval,
    struct timesyncpb_SyncEStatus *entry);

extern status_t build_to_xml_timesync_SyncSourceStatus (
    val_value_t *parentval,
    struct timesyncpb_SyncSourceStatus *entry);

extern status_t build_to_xml_timesync_SyncSourceInputStatusEntry (
    val_value_t *parentval,
    struct timesyncpb_SyncSourceInputStatusEntry *entry);


#endif /* _H_intri_timesync_trans */

