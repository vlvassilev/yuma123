// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_intri_system_trans
#define _H_intri_system_trans

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

extern status_t build_to_xml_system_Status (
    val_value_t *parentval,
    struct systempb_Status *entry);

extern status_t build_to_xml_system_Config (
    val_value_t *parentval,
    struct systempb_Config *entry);

extern status_t build_to_xml_system_IdentificationConfig (
    val_value_t *parentval,
    struct systempb_IdentificationConfig *entry);


#endif /* _H_intri_system_trans */

