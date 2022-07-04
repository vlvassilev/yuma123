// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_intri_files_trans
#define _H_intri_files_trans

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

extern status_t build_to_xml_files_Config(
    val_value_t *parentval,
    struct filespb_Config *entry);
extern status_t build_to_xml_files_ActivateCertificate(
    val_value_t *parentval,
    struct filespb_ActivateCertificate *entry);
extern status_t build_to_xml_files_CertificateID(
    val_value_t *parentval,
    struct filespb_CertificateID *entry);
extern status_t build_to_xml_files_CertificateIDList(
    val_value_t *parentval,
    struct filespb_CertificateIDList *entry);
extern status_t build_to_xml_files_CertificateData(
    val_value_t *parentval,
    struct filespb_CertificateData *entry);
extern status_t build_to_xml_files_CertificateUserID(
    val_value_t *parentval,
    struct filespb_CertificateUserID *entry);
extern status_t build_to_xml_files_CertificateUserIDList(
    val_value_t *parentval,
    struct filespb_CertificateUserIDList *entry);
extern status_t build_to_xml_files_CertificateType(
    val_value_t *parentval,
    struct filespb_CertificateType *entry);
extern status_t build_to_xml_files_CertificateInfo(
    val_value_t *parentval,
    struct filespb_CertificateInfo *entry);

extern status_t build_to_priv_files_Config(
    val_value_t *parentval,
    struct filespb_Config *entry);
extern status_t build_to_priv_files_ActivateCertificate(
    val_value_t *parentval,
    struct filespb_ActivateCertificate *entry);
extern status_t build_to_priv_files_CertificateID(
    val_value_t *parentval,
    struct filespb_CertificateID *entry);
extern status_t build_to_priv_files_CertificateIDList(
    val_value_t *parentval,
    struct filespb_CertificateIDList *entry);
extern status_t build_to_priv_files_CertificateData(
    val_value_t *parentval,
    struct filespb_CertificateData *entry);
extern status_t build_to_priv_files_CertificateUserID(
    val_value_t *parentval,
    struct filespb_CertificateUserID *entry);
extern status_t build_to_priv_files_CertificateUserIDList(
    val_value_t *parentval,
    struct filespb_CertificateUserIDList *entry);
extern status_t build_to_priv_files_CertificateType(
    val_value_t *parentval,
    struct filespb_CertificateType *entry);
extern status_t build_to_priv_files_CertificateInfo(
    val_value_t *parentval,
    struct filespb_CertificateInfo *entry);

#endif /* _H_intri_files_trans */

