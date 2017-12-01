#include "ncx.h"

/* base schema modules */
extern ncx_module_t  *yangcli_mod;
extern ncx_module_t  *yangcli_ex_mod;
extern ncx_module_t  *netconf_mod;

/* global connect param set, copied to server connect parmsets */
extern val_value_t   *connect_valset;

/* need to save CLI parameters: other vars are back-pointers */
extern val_value_t   *mgr_cli_valset;

/* Q of ncxmod_search_result_t structs representing all modules
 * and submodules found in the module library path at boot-time
 */
extern dlq_hdr_t      modlibQ;

extern ncxmod_temp_progcb_t *yangcli_progcb;
