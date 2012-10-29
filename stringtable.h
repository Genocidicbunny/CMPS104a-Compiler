#ifndef __STRINGTABLE_H__
#define __STRINGTABLE_H__

#include <stdio.h>
#include <stdint.h>
#include "auxlib.h"

typedef struct stringtable * stringtable_ref;
typedef struct stringnode * stringnode_ref;
typedef struct stringlist * stringlist_ref;

struct stringtable {
    uint32_t node_count;
    uint32_t dimension;
    uint32_t ops_count;
    uint32_t slots_occupied;
    float loadf;
    stringnode_ref *table_head;
};

struct stringnode {
    hashcode_t node_hash;
    cstring node_string;
    stringnode_ref next;
    stringnode_ref prev;
};

stringtable_ref table;

stringtable_ref new_stringtable(void);
void delete_stringtable(stringtable_ref);
void debugdump_stringtable(stringtable_ref, FILE*);
stringnode_ref intern_stringtable(stringtable_ref, cstring);
cstring peek_stringtable(stringnode_ref);
cstring repr_stringnode(stringnode_ref);
hashcode_t hashcode_stringtable(stringnode_ref);


#endif
