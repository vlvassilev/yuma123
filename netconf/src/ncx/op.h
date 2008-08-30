#ifndef _H_op
#define _H_op
/*  FILE: op.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

    NETCONF protocol operations

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
06-apr-05    abb      Begun.
*/

#include <xmlstring.h>

/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/



/********************************************************************
*                                                                   *
*                                    T Y P E S                      *
*                                                                   *
*********************************************************************/

/* NETCONF protocol operation enumeration is actually
 * an RPC method in the NECONF namespace
 */
typedef enum op_method_t_ {
    OP_NO_METHOD,
    OP_GET_CONFIG,            /* base protocol operations */
    OP_EDIT_CONFIG,
    OP_COPY_CONFIG,
    OP_DELETE_CONFIG,
    OP_LOCK,
    OP_UNLOCK,
    OP_GET,
    OP_CLOSE_SESSION,
    OP_KILL_SESSION,
    OP_COMMIT,                   /* #candidate capability */
    OP_DISCARD_CHANGES,          /* #candidate capability */
    OP_VALIDATE                  /* #validate capability */
} op_method_t;


/* NETCONF protocol operation PDU config types */
typedef enum op_config_t_ {
    OP_CONFIG_NONE,
    OP_CONFIG_CANDIDATE,  
    OP_CONFIG_RUNNING,
    OP_CONFIG_STARTUP
} op_config_t;


/* NETCONF protocol operation PDU source types */
typedef enum op_srctyp_t_ {
    OP_SOURCE_NONE,
    OP_SOURCE_CONFIG,  
    OP_SOURCE_INLINE,
    OP_SOURCE_URL
} op_srctyp_t;


/* NETCONF protocol operation PDU target types */
typedef enum op_targtyp_t_ {
    OP_TARGET_NONE,
    OP_TARGET_CONFIG,  
    OP_TARGET_URL
} op_targtyp_t;


/* NETCONF protocol default edit-config operation types */
typedef enum op_defop_t_ {
    OP_DEFOP_NONE,
    OP_DEFOP_MERGE,
    OP_DEFOP_REPLACE,
    OP_DEFOP_NODEF
} op_defop_t;


/* NETCONF protocol operation PDU filter types */
typedef enum op_filtertyp_t_ {
    OP_FILTER_NONE,
    OP_FILTER_SUBTREE,
    OP_FILTER_XPATH,
    OP_FILTER_C
} op_filtertyp_t;


/* NETCONF edit-config operation types */
typedef enum op_editop_t_ {
    OP_EDITOP_NONE,
    OP_EDITOP_MERGE,
    OP_EDITOP_REPLACE,
    OP_EDITOP_CREATE,
    OP_EDITOP_DELETE,
    OP_EDITOP_LOAD         /* internal enumeration */
} op_editop_t;


/* YANG insert operation types */
typedef enum op_insertop_t_ {
    OP_INSOP_NONE,
    OP_INSOP_FIRST,
    OP_INSOP_LAST,
    OP_INSOP_BEFORE,
    OP_INSOP_AFTER
} op_insertop_t;


/* NETCONF full operation list for access control */
typedef enum op_t_ {
    OP_NONE,
    OP_MERGE,
    OP_REPLACE,
    OP_CREATE,
    OP_DELETE,
    OP_LOAD,
    OP_NOTIFY,
    OP_READ
} op_t;


/* NETCONF edit-config test-option types */
typedef enum op_testop_t_ {
    OP_TESTOP_NONE,
    OP_TESTOP_TESTTHENSET,
    OP_TESTOP_SET,
    OP_TESTOP_TESTONLY
} op_testop_t;


/* NETCONF edit-config error-option types */
typedef enum op_errop_t_ {
    OP_ERROP_NONE,
    OP_ERROP_STOP,
    OP_ERROP_CONTINUE,
    OP_ERROP_ROLLBACK
} op_errop_t;


/* NETCONF protocol operation PDU sources union */
typedef union op_source_u_t_ {
    op_config_t     op_config;
    xmlChar        *op_inline;
    xmlChar        *op_url;
} op_source_u_t;


/* NETCONF protocol operation PDU source */
typedef struct op_source_t_ {
    op_srctyp_t    op_srctyp;
    op_source_u_t  op_src;
} op_source_t;


/* NETCONF protocol operation PDU targets union */
typedef union op_target_u_t_ {
    op_config_t     op_config;
    xmlChar        *op_url;
} op_target_u_t;


/* NETCONF protocol operation PDU target */
typedef struct op_target_t_ {
    op_targtyp_t    op_targtyp;
    op_target_u_t   op_targ;
} op_target_t;


/* NETCONF protocol operation filter spec */
typedef struct op_filter_t_ {
    op_filtertyp_t  op_filtyp;    
    void           *op_filter;  /* val_value_t w/ the filter value */
} op_filter_t;

 
/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

extern const xmlChar * 
    op_method_name (op_method_t op_id);

extern const xmlChar * 
    op_config_name (op_config_t cfg_id);

extern const xmlChar * 
    op_editop_name (op_editop_t ed_id);

extern op_editop_t 
    op_editop_id (const xmlChar *opstr);

extern const xmlChar * 
    op_insertop_name (op_insertop_t ins_id);

extern op_insertop_t 
    op_insertop_id (const xmlChar *opstr);

extern op_filtertyp_t 
    op_filtertyp_id (const xmlChar *filstr);

extern const xmlChar * 
    op_defop_name (op_defop_t def_id);

extern op_editop_t 
    op_defop_id (const xmlChar *defstr);

extern const xmlChar * 
    op_testop_name (op_testop_t test_id);

extern op_testop_t
    op_testop_enum (const xmlChar *teststr);

extern const xmlChar * 
    op_errop_name (op_errop_t err_id);

extern op_errop_t 
    op_errop_id (const xmlChar *errstr);


#endif            /* _H_op */
