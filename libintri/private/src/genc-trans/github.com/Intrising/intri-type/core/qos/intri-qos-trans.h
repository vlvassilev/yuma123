// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_intri_qos_trans
#define _H_intri_qos_trans

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

extern status_t build_to_xml_qos_WeightedFairTrafficRatioQueueEntry(
    val_value_t *parentval,
    struct qospb_WeightedFairTrafficRatioQueueEntry *entry);
extern status_t build_to_xml_qos_MappingCoSDot1PToQueueEntry(
    val_value_t *parentval,
    struct qospb_MappingCoSDot1PToQueueEntry *entry);
extern status_t build_to_xml_qos_MappingDSCPToQueueEntry(
    val_value_t *parentval,
    struct qospb_MappingDSCPToQueueEntry *entry);
extern status_t build_to_xml_qos_InterfaceConfigEntry(
    val_value_t *parentval,
    struct qospb_InterfaceConfigEntry *entry);
extern status_t build_to_xml_qos_Config(
    val_value_t *parentval,
    struct qospb_Config *entry);
extern status_t build_to_xml_qos_Mode(
    val_value_t *parentval,
    struct qospb_Mode *entry);
extern status_t build_to_xml_qos_TrustMode(
    val_value_t *parentval,
    struct qospb_TrustMode *entry);
extern status_t build_to_xml_qos_PriorityScheme(
    val_value_t *parentval,
    struct qospb_PriorityScheme *entry);
extern status_t build_to_xml_qos_QueueList(
    val_value_t *parentval,
    struct qospb_QueueList *entry);
extern status_t build_to_xml_qos_CoSList(
    val_value_t *parentval,
    struct qospb_CoSList *entry);
extern status_t build_to_xml_qos_DSCPList(
    val_value_t *parentval,
    struct qospb_DSCPList *entry);
extern status_t build_to_xml_qos_InterfaceList(
    val_value_t *parentval,
    struct qospb_InterfaceList *entry);

extern status_t build_to_priv_qos_WeightedFairTrafficRatioQueueEntry(
    val_value_t *parentval,
    struct qospb_WeightedFairTrafficRatioQueueEntry *entry);
extern status_t build_to_priv_qos_MappingCoSDot1PToQueueEntry(
    val_value_t *parentval,
    struct qospb_MappingCoSDot1PToQueueEntry *entry);
extern status_t build_to_priv_qos_MappingDSCPToQueueEntry(
    val_value_t *parentval,
    struct qospb_MappingDSCPToQueueEntry *entry);
extern status_t build_to_priv_qos_InterfaceConfigEntry(
    val_value_t *parentval,
    struct qospb_InterfaceConfigEntry *entry);
extern status_t build_to_priv_qos_Config(
    val_value_t *parentval,
    struct qospb_Config *entry);
extern status_t build_to_priv_qos_Mode(
    val_value_t *parentval,
    struct qospb_Mode *entry);
extern status_t build_to_priv_qos_TrustMode(
    val_value_t *parentval,
    struct qospb_TrustMode *entry);
extern status_t build_to_priv_qos_PriorityScheme(
    val_value_t *parentval,
    struct qospb_PriorityScheme *entry);
extern status_t build_to_priv_qos_QueueList(
    val_value_t *parentval,
    struct qospb_QueueList *entry);
extern status_t build_to_priv_qos_CoSList(
    val_value_t *parentval,
    struct qospb_CoSList *entry);
extern status_t build_to_priv_qos_DSCPList(
    val_value_t *parentval,
    struct qospb_DSCPList *entry);
extern status_t build_to_priv_qos_InterfaceList(
    val_value_t *parentval,
    struct qospb_InterfaceList *entry);

#endif /* _H_intri_qos_trans */

