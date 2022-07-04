// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_intri_annotations_trans
#define _H_intri_annotations_trans

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

extern status_t build_to_xml_annotations_Http(
    val_value_t *parentval,
    struct annotationspb_Http *entry);
extern status_t build_to_xml_annotations_HttpRule(
    val_value_t *parentval,
    struct annotationspb_HttpRule *entry);
extern status_t build_to_xml_annotations_CustomHttpPattern(
    val_value_t *parentval,
    struct annotationspb_CustomHttpPattern *entry);

extern status_t build_to_priv_annotations_Http(
    val_value_t *parentval,
    struct annotationspb_Http *entry);
extern status_t build_to_priv_annotations_HttpRule(
    val_value_t *parentval,
    struct annotationspb_HttpRule *entry);
extern status_t build_to_priv_annotations_CustomHttpPattern(
    val_value_t *parentval,
    struct annotationspb_CustomHttpPattern *entry);

#endif /* _H_intri_annotations_trans */

