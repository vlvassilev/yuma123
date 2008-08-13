/*  FILE: agt_val_util.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
19dec05      abb      begun
21jul08      abb      start obj-based rewrite
08aug08      abb      clone for agent <rpc-error> reporting version

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <memory.h>
#include  <string.h>
#include  <ctype.h>

#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_val_util
#include "agt_val_util.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_val_util
#include "val_util.h"
#endif

#ifndef _H_xml_msg
#include "xml_msg.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define AGT_VAL_UTIL_DEBUG  1
#endif


/*************** E X T E R N A L    F U N C T I O N S  *************/





/* END file agt_val_util.c */
