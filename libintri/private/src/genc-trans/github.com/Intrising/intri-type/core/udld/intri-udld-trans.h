// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_intri_udld_trans
#define _H_intri_udld_trans

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

extern status_t build_to_xml_udld_Config(
    val_value_t *parentval,
    struct udldpb_Config *entry);
extern status_t build_to_xml_udld_BasicConfig(
    val_value_t *parentval,
    struct udldpb_BasicConfig *entry);
extern status_t build_to_xml_udld_PortConfig(
    val_value_t *parentval,
    struct udldpb_PortConfig *entry);
extern status_t build_to_xml_udld_PortConfigEntry(
    val_value_t *parentval,
    struct udldpb_PortConfigEntry *entry);
extern status_t build_to_xml_udld_Status(
    val_value_t *parentval,
    struct udldpb_Status *entry);
extern status_t build_to_xml_udld_PortStatus(
    val_value_t *parentval,
    struct udldpb_PortStatus *entry);
extern status_t build_to_xml_udld_PortStatusEntry(
    val_value_t *parentval,
    struct udldpb_PortStatusEntry *entry);
extern status_t build_to_xml_udld_NeighborStatus(
    val_value_t *parentval,
    struct udldpb_NeighborStatus *entry);
extern status_t build_to_xml_udld_NeighborStatusEntry(
    val_value_t *parentval,
    struct udldpb_NeighborStatusEntry *entry);
extern status_t build_to_xml_udld_Statistics(
    val_value_t *parentval,
    struct udldpb_Statistics *entry);
extern status_t build_to_xml_udld_PortStatistics(
    val_value_t *parentval,
    struct udldpb_PortStatistics *entry);
extern status_t build_to_xml_udld_PortStatisticsEntry(
    val_value_t *parentval,
    struct udldpb_PortStatisticsEntry *entry);
extern status_t build_to_xml_udld_PacketStatistics(
    val_value_t *parentval,
    struct udldpb_PacketStatistics *entry);
extern status_t build_to_xml_udld_PacketStatisticsEntry(
    val_value_t *parentval,
    struct udldpb_PacketStatisticsEntry *entry);

extern status_t build_to_priv_udld_Config(
    val_value_t *parentval,
    struct udldpb_Config *entry);
extern status_t build_to_priv_udld_BasicConfig(
    val_value_t *parentval,
    struct udldpb_BasicConfig *entry);
extern status_t build_to_priv_udld_PortConfig(
    val_value_t *parentval,
    struct udldpb_PortConfig *entry);
extern status_t build_to_priv_udld_PortConfigEntry(
    val_value_t *parentval,
    struct udldpb_PortConfigEntry *entry);
extern status_t build_to_priv_udld_Status(
    val_value_t *parentval,
    struct udldpb_Status *entry);
extern status_t build_to_priv_udld_PortStatus(
    val_value_t *parentval,
    struct udldpb_PortStatus *entry);
extern status_t build_to_priv_udld_PortStatusEntry(
    val_value_t *parentval,
    struct udldpb_PortStatusEntry *entry);
extern status_t build_to_priv_udld_NeighborStatus(
    val_value_t *parentval,
    struct udldpb_NeighborStatus *entry);
extern status_t build_to_priv_udld_NeighborStatusEntry(
    val_value_t *parentval,
    struct udldpb_NeighborStatusEntry *entry);
extern status_t build_to_priv_udld_Statistics(
    val_value_t *parentval,
    struct udldpb_Statistics *entry);
extern status_t build_to_priv_udld_PortStatistics(
    val_value_t *parentval,
    struct udldpb_PortStatistics *entry);
extern status_t build_to_priv_udld_PortStatisticsEntry(
    val_value_t *parentval,
    struct udldpb_PortStatisticsEntry *entry);
extern status_t build_to_priv_udld_PacketStatistics(
    val_value_t *parentval,
    struct udldpb_PacketStatistics *entry);
extern status_t build_to_priv_udld_PacketStatisticsEntry(
    val_value_t *parentval,
    struct udldpb_PacketStatisticsEntry *entry);

#endif /* _H_intri_udld_trans */

