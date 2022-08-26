// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_sercom_dhcp_trans
#define _H_sercom_dhcp_trans

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

extern status_t build_to_xml_dhcp_ARPInspectionConfig(
    val_value_t *parentval,
    struct dhcppb_ARPInspectionConfig *entry);
extern status_t build_to_xml_dhcp_ARPInspectionPortEntry(
    val_value_t *parentval,
    struct dhcppb_ARPInspectionPortEntry *entry);
extern status_t build_to_xml_dhcp_ARPInspectionACLConfig(
    val_value_t *parentval,
    struct dhcppb_ARPInspectionACLConfig *entry);
extern status_t build_to_xml_dhcp_ARPInspectionACLRuleEntry(
    val_value_t *parentval,
    struct dhcppb_ARPInspectionACLRuleEntry *entry);
extern status_t build_to_xml_dhcp_Config(
    val_value_t *parentval,
    struct dhcppb_Config *entry);
extern status_t build_to_xml_dhcp_RelayConfig(
    val_value_t *parentval,
    struct dhcppb_RelayConfig *entry);
extern status_t build_to_xml_dhcp_ConfigRelayPortEntry(
    val_value_t *parentval,
    struct dhcppb_ConfigRelayPortEntry *entry);
extern status_t build_to_xml_dhcp_SnoopingConfig(
    val_value_t *parentval,
    struct dhcppb_SnoopingConfig *entry);
extern status_t build_to_xml_dhcp_SnoopingConfigPortEntry(
    val_value_t *parentval,
    struct dhcppb_SnoopingConfigPortEntry *entry);
extern status_t build_to_xml_dhcp_SnoopingStatisticsEntry(
    val_value_t *parentval,
    struct dhcppb_SnoopingStatisticsEntry *entry);
extern status_t build_to_xml_dhcp_SnoopingStatisticsList(
    val_value_t *parentval,
    struct dhcppb_SnoopingStatisticsList *entry);
extern status_t build_to_xml_dhcp_SnoopingBindingDatabaseEntry(
    val_value_t *parentval,
    struct dhcppb_SnoopingBindingDatabaseEntry *entry);
extern status_t build_to_xml_dhcp_SnoopingBindingDatabaseList(
    val_value_t *parentval,
    struct dhcppb_SnoopingBindingDatabaseList *entry);

extern status_t build_to_priv_dhcp_ARPInspectionConfig(
    val_value_t *parentval,
    struct dhcppb_ARPInspectionConfig *entry);
extern status_t build_to_priv_dhcp_ARPInspectionPortEntry(
    val_value_t *parentval,
    struct dhcppb_ARPInspectionPortEntry *entry);
extern status_t build_to_priv_dhcp_ARPInspectionACLConfig(
    val_value_t *parentval,
    struct dhcppb_ARPInspectionACLConfig *entry);
extern status_t build_to_priv_dhcp_ARPInspectionACLRuleEntry(
    val_value_t *parentval,
    struct dhcppb_ARPInspectionACLRuleEntry *entry);
extern status_t build_to_priv_dhcp_Config(
    val_value_t *parentval,
    struct dhcppb_Config *entry);
extern status_t build_to_priv_dhcp_RelayConfig(
    val_value_t *parentval,
    struct dhcppb_RelayConfig *entry);
extern status_t build_to_priv_dhcp_ConfigRelayPortEntry(
    val_value_t *parentval,
    struct dhcppb_ConfigRelayPortEntry *entry);
extern status_t build_to_priv_dhcp_SnoopingConfig(
    val_value_t *parentval,
    struct dhcppb_SnoopingConfig *entry);
extern status_t build_to_priv_dhcp_SnoopingConfigPortEntry(
    val_value_t *parentval,
    struct dhcppb_SnoopingConfigPortEntry *entry);
extern status_t build_to_priv_dhcp_SnoopingStatisticsEntry(
    val_value_t *parentval,
    struct dhcppb_SnoopingStatisticsEntry *entry);
extern status_t build_to_priv_dhcp_SnoopingStatisticsList(
    val_value_t *parentval,
    struct dhcppb_SnoopingStatisticsList *entry);
extern status_t build_to_priv_dhcp_SnoopingBindingDatabaseEntry(
    val_value_t *parentval,
    struct dhcppb_SnoopingBindingDatabaseEntry *entry);
extern status_t build_to_priv_dhcp_SnoopingBindingDatabaseList(
    val_value_t *parentval,
    struct dhcppb_SnoopingBindingDatabaseList *entry);

#endif /* _H_sercom_dhcp_trans */

