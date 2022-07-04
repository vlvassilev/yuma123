// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_intri_loop_trans
#define _H_intri_loop_trans

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

extern status_t build_to_xml_loop_Config(
    val_value_t *parentval,
    struct looppb_Config *entry);
extern status_t build_to_xml_loop_PortConfigEntry(
    val_value_t *parentval,
    struct looppb_PortConfigEntry *entry);
extern status_t build_to_xml_loop_PortConfig(
    val_value_t *parentval,
    struct looppb_PortConfig *entry);
extern status_t build_to_xml_loop_StatusEntry(
    val_value_t *parentval,
    struct looppb_StatusEntry *entry);
extern status_t build_to_xml_loop_Status(
    val_value_t *parentval,
    struct looppb_Status *entry);

extern status_t build_to_priv_loop_Config(
    val_value_t *parentval,
    struct looppb_Config *entry);
extern status_t build_to_priv_loop_PortConfigEntry(
    val_value_t *parentval,
    struct looppb_PortConfigEntry *entry);
extern status_t build_to_priv_loop_PortConfig(
    val_value_t *parentval,
    struct looppb_PortConfig *entry);
extern status_t build_to_priv_loop_StatusEntry(
    val_value_t *parentval,
    struct looppb_StatusEntry *entry);
extern status_t build_to_priv_loop_Status(
    val_value_t *parentval,
    struct looppb_Status *entry);

#endif /* _H_intri_loop_trans */

