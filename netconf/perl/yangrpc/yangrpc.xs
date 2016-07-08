#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

static int yangrpc_init_done = 0;

MODULE = yangrpc		PACKAGE = yangrpc		

void* connect(char* server, U16 port, char* user, char* password, char* public_key, char* private_key)
	CODE:
	if(yangrpc_init_done==0) {
		yangrpc_init(0,NULL);
		yangrpc_init_done=1;
	}
	RETVAL = (void*)yangrpc_connect(server, port, user, password, public_key, private_key);

	OUTPUT:
	RETVAL

