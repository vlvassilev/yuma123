#include <string.h>
#include <assert.h>
#include "dlq.h"
#include "dict.h"

void dict_init(dlq_hdr_t *que) {
    dlq_createSQue(que);
}

void dict_clear(dlq_hdr_t *que)
{
    dict_node_t *dn;
    while (!dlq_empty(que)) {
        dn = (dict_node_t *)dlq_deque(que);
        free(dn);
    }
    dlq_createSQue(que);
}

char* dict_get_name(dlq_hdr_t *que, void* data)
{
    dict_node_t *dn;
    for (dn = (dict_node_t *)dlq_firstEntry(que);
         dn != NULL;
         dn = (dict_node_t *)dlq_nextEntry(dn)) {
        if(dn->data==data) {
            return dn->name;
        }
    }
    return NULL;
}

void* dict_get_data(dlq_hdr_t *que, const char* name)
{
    dict_node_t *dn;
    for (dn = (dict_node_t *)dlq_firstEntry(que);
         dn != NULL;
         dn = (dict_node_t *)dlq_nextEntry(dn)) {
        if(0==strcmp(dn->name, name)) {
            return dn->data;
        }
    }
    assert(0);
}

void dict_add(dlq_hdr_t *que, const char* name, void* data)
{
    dict_node_t *dn = malloc(sizeof(dict_node_t));
    assert(dn);
    dn->data=data;
    dn->name=(char*)name;
    dlq_enque (dn, que);
}

void dict_remove(dlq_hdr_t *que, const char* name)
{
    dict_node_t *dn;
    for (dn = (dict_node_t *)dlq_firstEntry(que);
         dn != NULL;
         dn = (dict_node_t *)dlq_nextEntry(dn)) {
        if(0==strcmp(dn->name,name)) {
            dlq_remove (dn);
            free(dn);     
            return;
        }
    }
    assert(0);
}
