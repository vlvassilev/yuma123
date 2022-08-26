// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_sercom_rmon
#define _H_sercom_rmon

/*****************************************************************************************************
 * Copyright (C) 2017-2022 by Intrising
 *  - ian0113@intrising.com.tw
 * 
 * Generated by protoc-gen-yang@gen-yang
 * 
 *****************************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <libxml/xmlstring.h>
#include "dlq.h"
#include "ncxtypes.h"
#include "op.h"
#include "status.h"
#include "val.h"

#define y_M_intri_rmon (const xmlChar *)"sercom-rmon"
#define y_R_intri_rmon (const xmlChar *)"2022-06-24"

/*****************************************************************************************************
 * FUNCTION y_sercom_rmon_init
 * 
 * initialize the sercom-rmon server instrumentation library
 * 
 * INPUTS:
 *   modname == requested module name
 *   revision == requested version (NULL for any)
 * 
 * RETURNS:
 *   error status
 *****************************************************************************************************/
extern status_t y_sercom_rmon_init(
    const xmlChar *modname,
    const xmlChar *revision);

/*****************************************************************************************************
 * FUNCTION y_sercom_rmon_init2
 * 
 * SIL init phase 2: non-config data structures
 * Called after running config is loaded
 * 
 * RETURNS:
 *   error status
 *****************************************************************************************************/
extern status_t y_sercom_rmon_init2(void);

/*****************************************************************************************************
 * FUNCTION y_sercom_rmon_cleanup
 * 
 * cleanup the server instrumentation library
 *****************************************************************************************************/
extern void y_sercom_rmon_cleanup(void);

#ifdef __cplusplus
} /* end extern 'C' */
#endif

#endif /* _H_sercom_rmon */
