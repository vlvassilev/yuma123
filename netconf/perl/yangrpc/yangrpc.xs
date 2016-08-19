#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

static int yangrpc_init_done = 0;

MODULE = yangrpc		PACKAGE = yangrpc		

void* connect(char* server, U16 port, char* user, char* password, char* public_key, char* private_key)
	CODE:
	void* yangrpc_cb_ptr;
	if(yangrpc_init_done==0) {
		yangrpc_init(NULL);
		yangrpc_init_done=1;
	}
	yangrpc_connect(server, port, user, password, public_key, private_key, (char*)NULL, &yangrpc_cb_ptr);
	RETVAL = yangrpc_cb_ptr;
	OUTPUT:
	RETVAL

int parse_cli(void* yangrpc_cb_ptr, char* cli_cmd, void* &rpc_val)
	CODE:
	RETVAL=yangrpc_parse_cli(yangrpc_cb_ptr, cli_cmd, &rpc_val);
	OUTPUT:
	rpc_val
	RETVAL

int rpc(void* yangrpc_cb_ptr, void* rpc_val, void* &reply_val)
	CODE:
	RETVAL = yangrpc_exec(yangrpc_cb_ptr, rpc_val, &reply_val);
	OUTPUT:
	reply_val
	RETVAL

