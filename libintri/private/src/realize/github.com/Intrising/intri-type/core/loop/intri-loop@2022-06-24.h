// Code generated by protoc-gen-yang(*.h) DO NOT EDIT.

#ifndef _H_intri_loop
#define _H_intri_loop

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

#define y_M_intri_loop (const xmlChar *)"intri-loop"
#define y_R_intri_loop (const xmlChar *)"2022-06-24"

/*****************************************************************************************************
 * FUNCTION y_intri_loop_init
 * 
 * initialize the intri-loop server instrumentation library
 * 
 * INPUTS:
 *   modname == requested module name
 *   revision == requested version (NULL for any)
 * 
 * RETURNS:
 *   error status
 *****************************************************************************************************/
extern status_t y_intri_loop_init(
    const xmlChar *modname,
    const xmlChar *revision);

/*****************************************************************************************************
 * FUNCTION y_intri_loop_init2
 * 
 * SIL init phase 2: non-config data structures
 * Called after running config is loaded
 * 
 * RETURNS:
 *   error status
 *****************************************************************************************************/
extern status_t y_intri_loop_init2(void);

/*****************************************************************************************************
 * FUNCTION y_intri_loop_cleanup
 * 
 * cleanup the server instrumentation library
 *****************************************************************************************************/
extern void y_intri_loop_cleanup(void);

#ifdef __cplusplus
} /* end extern 'C' */
#endif

#endif /* _H_intri_loop */
