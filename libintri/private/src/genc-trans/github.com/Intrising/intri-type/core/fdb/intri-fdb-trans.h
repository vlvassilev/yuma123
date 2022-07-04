// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_intri_fdb_trans
#define _H_intri_fdb_trans

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

extern status_t build_to_xml_fdb_Config(
    val_value_t *parentval,
    struct fdbpb_Config *entry);
extern status_t build_to_xml_fdb_AgingTime(
    val_value_t *parentval,
    struct fdbpb_AgingTime *entry);
extern status_t build_to_xml_fdb_ForwardConfig(
    val_value_t *parentval,
    struct fdbpb_ForwardConfig *entry);
extern status_t build_to_xml_fdb_ForwardConfigEntry(
    val_value_t *parentval,
    struct fdbpb_ForwardConfigEntry *entry);
extern status_t build_to_xml_fdb_DropConfig(
    val_value_t *parentval,
    struct fdbpb_DropConfig *entry);
extern status_t build_to_xml_fdb_DropConfigEntry(
    val_value_t *parentval,
    struct fdbpb_DropConfigEntry *entry);
extern status_t build_to_xml_fdb_PortLearningLimit(
    val_value_t *parentval,
    struct fdbpb_PortLearningLimit *entry);
extern status_t build_to_xml_fdb_PortLearningLimitEntry(
    val_value_t *parentval,
    struct fdbpb_PortLearningLimitEntry *entry);
extern status_t build_to_xml_fdb_Info(
    val_value_t *parentval,
    struct fdbpb_Info *entry);
extern status_t build_to_xml_fdb_Status(
    val_value_t *parentval,
    struct fdbpb_Status *entry);
extern status_t build_to_xml_fdb_StatusEntry(
    val_value_t *parentval,
    struct fdbpb_StatusEntry *entry);
extern status_t build_to_xml_fdb_SpecificMac(
    val_value_t *parentval,
    struct fdbpb_SpecificMac *entry);
extern status_t build_to_xml_fdb_FlushOption(
    val_value_t *parentval,
    struct fdbpb_FlushOption *entry);
extern status_t build_to_xml_fdb_PortOccupied(
    val_value_t *parentval,
    struct fdbpb_PortOccupied *entry);

extern status_t build_to_priv_fdb_Config(
    val_value_t *parentval,
    struct fdbpb_Config *entry);
extern status_t build_to_priv_fdb_AgingTime(
    val_value_t *parentval,
    struct fdbpb_AgingTime *entry);
extern status_t build_to_priv_fdb_ForwardConfig(
    val_value_t *parentval,
    struct fdbpb_ForwardConfig *entry);
extern status_t build_to_priv_fdb_ForwardConfigEntry(
    val_value_t *parentval,
    struct fdbpb_ForwardConfigEntry *entry);
extern status_t build_to_priv_fdb_DropConfig(
    val_value_t *parentval,
    struct fdbpb_DropConfig *entry);
extern status_t build_to_priv_fdb_DropConfigEntry(
    val_value_t *parentval,
    struct fdbpb_DropConfigEntry *entry);
extern status_t build_to_priv_fdb_PortLearningLimit(
    val_value_t *parentval,
    struct fdbpb_PortLearningLimit *entry);
extern status_t build_to_priv_fdb_PortLearningLimitEntry(
    val_value_t *parentval,
    struct fdbpb_PortLearningLimitEntry *entry);
extern status_t build_to_priv_fdb_Info(
    val_value_t *parentval,
    struct fdbpb_Info *entry);
extern status_t build_to_priv_fdb_Status(
    val_value_t *parentval,
    struct fdbpb_Status *entry);
extern status_t build_to_priv_fdb_StatusEntry(
    val_value_t *parentval,
    struct fdbpb_StatusEntry *entry);
extern status_t build_to_priv_fdb_SpecificMac(
    val_value_t *parentval,
    struct fdbpb_SpecificMac *entry);
extern status_t build_to_priv_fdb_FlushOption(
    val_value_t *parentval,
    struct fdbpb_FlushOption *entry);
extern status_t build_to_priv_fdb_PortOccupied(
    val_value_t *parentval,
    struct fdbpb_PortOccupied *entry);

#endif /* _H_intri_fdb_trans */

