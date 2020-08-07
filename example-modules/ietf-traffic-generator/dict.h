#include "dlq.h"

typedef struct dict_node_t_ {
    dlq_hdr_t      qhdr;
    char* name;
    void* data;
} dict_node_t;


void dict_init(dlq_hdr_t *que);
void dict_clear(dlq_hdr_t *que);
char* dict_get_name(dlq_hdr_t *que, void* data);
void* dict_get_data(dlq_hdr_t *que, const char* name);
void dict_add(dlq_hdr_t *que, const char* name, void* data);
void dict_remove(dlq_hdr_t *que, const char* name);
