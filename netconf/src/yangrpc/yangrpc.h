#include "val.h"

typedef struct dummy {int dummy;} yangrpc_cb_t;

yangrpc_cb_t* yangrpc_connect(char* server, char* user, char* password, char* publick_key, char* private_key);
status_t yangrpc_exec(yangrpc_cb_t *server_cb, val_value_t* request_val, val_value_t** reply_val);
void yangrpc_close(yangrpc_cb_t *yangrpc_cb);
