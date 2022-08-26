// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_sercom_lldp_trans
#define _H_sercom_lldp_trans

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

extern status_t build_to_xml_lldp_Config(
    val_value_t *parentval,
    struct lldppb_Config *entry);
extern status_t build_to_xml_lldp_SystemConfig(
    val_value_t *parentval,
    struct lldppb_SystemConfig *entry);
extern status_t build_to_xml_lldp_PortConfig(
    val_value_t *parentval,
    struct lldppb_PortConfig *entry);
extern status_t build_to_xml_lldp_PortConfigEntry(
    val_value_t *parentval,
    struct lldppb_PortConfigEntry *entry);
extern status_t build_to_xml_lldp_LocalInfo(
    val_value_t *parentval,
    struct lldppb_LocalInfo *entry);
extern status_t build_to_xml_lldp_NeighborInfo(
    val_value_t *parentval,
    struct lldppb_NeighborInfo *entry);
extern status_t build_to_xml_lldp_LocalInfoEntry(
    val_value_t *parentval,
    struct lldppb_LocalInfoEntry *entry);
extern status_t build_to_xml_lldp_NeighborInfoEntry(
    val_value_t *parentval,
    struct lldppb_NeighborInfoEntry *entry);
extern status_t build_to_xml_lldp_LinkAggregation(
    val_value_t *parentval,
    struct lldppb_LinkAggregation *entry);
extern status_t build_to_xml_lldp_ExtendedPowerViaMDI(
    val_value_t *parentval,
    struct lldppb_ExtendedPowerViaMDI *entry);
extern status_t build_to_xml_lldp_MACPHYConfig(
    val_value_t *parentval,
    struct lldppb_MACPHYConfig *entry);
extern status_t build_to_xml_lldp_MediaCapability(
    val_value_t *parentval,
    struct lldppb_MediaCapability *entry);
extern status_t build_to_xml_lldp_SystemManagementInfo(
    val_value_t *parentval,
    struct lldppb_SystemManagementInfo *entry);
extern status_t build_to_xml_lldp_PortManagementInfo(
    val_value_t *parentval,
    struct lldppb_PortManagementInfo *entry);
extern status_t build_to_xml_lldp_PowerViaMDI(
    val_value_t *parentval,
    struct lldppb_PowerViaMDI *entry);
extern status_t build_to_xml_lldp_PortManagementAddressInfo(
    val_value_t *parentval,
    struct lldppb_PortManagementAddressInfo *entry);
extern status_t build_to_xml_lldp_ManagementAddress(
    val_value_t *parentval,
    struct lldppb_ManagementAddress *entry);
extern status_t build_to_xml_lldp_ChassisInfo(
    val_value_t *parentval,
    struct lldppb_ChassisInfo *entry);
extern status_t build_to_xml_lldp_PortID(
    val_value_t *parentval,
    struct lldppb_PortID *entry);
extern status_t build_to_xml_lldp_Statistic(
    val_value_t *parentval,
    struct lldppb_Statistic *entry);
extern status_t build_to_xml_lldp_StatisticEntry(
    val_value_t *parentval,
    struct lldppb_StatisticEntry *entry);
extern status_t build_to_xml_lldp_VoiceVlanEntry(
    val_value_t *parentval,
    struct lldppb_VoiceVlanEntry *entry);

extern status_t build_to_priv_lldp_Config(
    val_value_t *parentval,
    struct lldppb_Config *entry);
extern status_t build_to_priv_lldp_SystemConfig(
    val_value_t *parentval,
    struct lldppb_SystemConfig *entry);
extern status_t build_to_priv_lldp_PortConfig(
    val_value_t *parentval,
    struct lldppb_PortConfig *entry);
extern status_t build_to_priv_lldp_PortConfigEntry(
    val_value_t *parentval,
    struct lldppb_PortConfigEntry *entry);
extern status_t build_to_priv_lldp_LocalInfo(
    val_value_t *parentval,
    struct lldppb_LocalInfo *entry);
extern status_t build_to_priv_lldp_NeighborInfo(
    val_value_t *parentval,
    struct lldppb_NeighborInfo *entry);
extern status_t build_to_priv_lldp_LocalInfoEntry(
    val_value_t *parentval,
    struct lldppb_LocalInfoEntry *entry);
extern status_t build_to_priv_lldp_NeighborInfoEntry(
    val_value_t *parentval,
    struct lldppb_NeighborInfoEntry *entry);
extern status_t build_to_priv_lldp_LinkAggregation(
    val_value_t *parentval,
    struct lldppb_LinkAggregation *entry);
extern status_t build_to_priv_lldp_ExtendedPowerViaMDI(
    val_value_t *parentval,
    struct lldppb_ExtendedPowerViaMDI *entry);
extern status_t build_to_priv_lldp_MACPHYConfig(
    val_value_t *parentval,
    struct lldppb_MACPHYConfig *entry);
extern status_t build_to_priv_lldp_MediaCapability(
    val_value_t *parentval,
    struct lldppb_MediaCapability *entry);
extern status_t build_to_priv_lldp_SystemManagementInfo(
    val_value_t *parentval,
    struct lldppb_SystemManagementInfo *entry);
extern status_t build_to_priv_lldp_PortManagementInfo(
    val_value_t *parentval,
    struct lldppb_PortManagementInfo *entry);
extern status_t build_to_priv_lldp_PowerViaMDI(
    val_value_t *parentval,
    struct lldppb_PowerViaMDI *entry);
extern status_t build_to_priv_lldp_PortManagementAddressInfo(
    val_value_t *parentval,
    struct lldppb_PortManagementAddressInfo *entry);
extern status_t build_to_priv_lldp_ManagementAddress(
    val_value_t *parentval,
    struct lldppb_ManagementAddress *entry);
extern status_t build_to_priv_lldp_ChassisInfo(
    val_value_t *parentval,
    struct lldppb_ChassisInfo *entry);
extern status_t build_to_priv_lldp_PortID(
    val_value_t *parentval,
    struct lldppb_PortID *entry);
extern status_t build_to_priv_lldp_Statistic(
    val_value_t *parentval,
    struct lldppb_Statistic *entry);
extern status_t build_to_priv_lldp_StatisticEntry(
    val_value_t *parentval,
    struct lldppb_StatisticEntry *entry);
extern status_t build_to_priv_lldp_VoiceVlanEntry(
    val_value_t *parentval,
    struct lldppb_VoiceVlanEntry *entry);

#endif /* _H_sercom_lldp_trans */

