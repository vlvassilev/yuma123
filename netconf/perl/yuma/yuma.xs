#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"


MODULE = yuma		PACKAGE = yuma

int val_make_serialized_string(void* val, int mode, char* xml_str)
	CODE:
	RETVAL = val_make_serialized_string(val, mode, &xml_str);
	OUTPUT:
	xml_str
	RETVAL

