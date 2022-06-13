// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_intri_acl_trans
#define _H_intri_acl_trans

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

extern status_t build_to_xml_acl_Config (
    val_value_t *parentval,
    struct aclpb_Config *entry);

extern status_t build_to_xml_acl_InterfaceList (
    val_value_t *parentval,
    struct aclpb_InterfaceList *entry);

extern status_t build_to_xml_acl_InterfaceEntry (
    val_value_t *parentval,
    struct aclpb_InterfaceEntry *entry);

extern status_t build_to_xml_acl_ACLList (
    val_value_t *parentval,
    struct aclpb_ACLList *entry);

extern status_t build_to_xml_acl_ACLEntry (
    val_value_t *parentval,
    struct aclpb_ACLEntry *entry);

extern status_t build_to_xml_acl_ACEList (
    val_value_t *parentval,
    struct aclpb_ACEList *entry);

extern status_t build_to_xml_acl_ACEEntry (
    val_value_t *parentval,
    struct aclpb_ACEEntry *entry);

extern status_t build_to_xml_acl_RuleVlan (
    val_value_t *parentval,
    struct aclpb_RuleVlan *entry);

extern status_t build_to_xml_acl_RuleMACIPv4 (
    val_value_t *parentval,
    struct aclpb_RuleMACIPv4 *entry);

extern status_t build_to_xml_acl_RuleMACIPv6 (
    val_value_t *parentval,
    struct aclpb_RuleMACIPv6 *entry);

extern status_t build_to_xml_acl_RuleMAC (
    val_value_t *parentval,
    struct aclpb_RuleMAC *entry);

extern status_t build_to_xml_acl_MACConfig (
    val_value_t *parentval,
    struct aclpb_MACConfig *entry);

extern status_t build_to_xml_acl_EtherTypeConfig (
    val_value_t *parentval,
    struct aclpb_EtherTypeConfig *entry);

extern status_t build_to_xml_acl_IPProtocolConfig (
    val_value_t *parentval,
    struct aclpb_IPProtocolConfig *entry);

extern status_t build_to_xml_acl_RuleIPv4 (
    val_value_t *parentval,
    struct aclpb_RuleIPv4 *entry);

extern status_t build_to_xml_acl_IPv4Config (
    val_value_t *parentval,
    struct aclpb_IPv4Config *entry);

extern status_t build_to_xml_acl_RuleIPv6 (
    val_value_t *parentval,
    struct aclpb_RuleIPv6 *entry);

extern status_t build_to_xml_acl_IPv6Config (
    val_value_t *parentval,
    struct aclpb_IPv6Config *entry);

extern status_t build_to_xml_acl_RuleLayer4Port (
    val_value_t *parentval,
    struct aclpb_RuleLayer4Port *entry);

extern status_t build_to_xml_acl_IPWithLayer4PortConfig (
    val_value_t *parentval,
    struct aclpb_IPWithLayer4PortConfig *entry);

extern status_t build_to_xml_acl_BindingList (
    val_value_t *parentval,
    struct aclpb_BindingList *entry);

extern status_t build_to_xml_acl_BindingEntry (
    val_value_t *parentval,
    struct aclpb_BindingEntry *entry);

extern status_t build_to_xml_acl_FlowMirroringEntry (
    val_value_t *parentval,
    struct aclpb_FlowMirroringEntry *entry);

extern status_t build_to_xml_acl_FlowMirroringList (
    val_value_t *parentval,
    struct aclpb_FlowMirroringList *entry);


#endif /* _H_intri_acl_trans */
