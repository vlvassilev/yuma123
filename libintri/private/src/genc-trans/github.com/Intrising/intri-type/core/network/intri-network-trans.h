// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_intri_network_trans
#define _H_intri_network_trans

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

extern status_t build_to_xml_network_Config (
    val_value_t *parentval,
    struct networkpb_Config *entry);

extern status_t build_to_xml_network_BasicConfig (
    val_value_t *parentval,
    struct networkpb_BasicConfig *entry);

extern status_t build_to_xml_network_IPConfig (
    val_value_t *parentval,
    struct networkpb_IPConfig *entry);

extern status_t build_to_xml_network_IPv4Config (
    val_value_t *parentval,
    struct networkpb_IPv4Config *entry);

extern status_t build_to_xml_network_IPv4Static (
    val_value_t *parentval,
    struct networkpb_IPv4Static *entry);

extern status_t build_to_xml_network_IPv6Config (
    val_value_t *parentval,
    struct networkpb_IPv6Config *entry);

extern status_t build_to_xml_network_IPv6Static (
    val_value_t *parentval,
    struct networkpb_IPv6Static *entry);

extern status_t build_to_xml_network_IPv4Status (
    val_value_t *parentval,
    struct networkpb_IPv4Status *entry);

extern status_t build_to_xml_network_IPv6StatusEntry (
    val_value_t *parentval,
    struct networkpb_IPv6StatusEntry *entry);

extern status_t build_to_xml_network_IPv6Status (
    val_value_t *parentval,
    struct networkpb_IPv6Status *entry);


#endif /* _H_intri_network_trans */

