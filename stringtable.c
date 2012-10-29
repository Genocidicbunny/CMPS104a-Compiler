#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stringtable.h"
#include "strhash.h"
#include "auxlib.h"

// typedef char * cstring;
// typedef uint32_t hashcode_t;
// typedef struct stringtable * stringtable_ref;
// typedef struct stringnode * stringnode_ref;
#define BASE_DIMENSION          31
#define LOAD_MAX                0.6
#define COLLISION_CHAIN_MAX     5
#define REHASH_OPS_THRESHOLD    10



stringtable_ref new_stringtable(void);
void delete_stringtable(stringtable_ref);
void debugdump_stringtable(stringtable_ref, FILE*);
stringnode_ref intern_stringtable(stringtable_ref, cstring);
cstring peek_stringtable(stringnode_ref);
cstring repr_stringnode(stringnode_ref);
hashcode_t hashcode_stringtable(stringnode_ref);

void delete_node(stringnode_ref);
void delete_node_chain(stringnode_ref);
stringnode_ref find_chain_tail(stringnode_ref);
stringnode_ref find_node(stringnode_ref *, uint32_t, hashcode_t);
stringnode_ref find_node_from_chain(stringnode_ref, hashcode_t);
void debugdump_node_chain(stringnode_ref, FILE *, unsigned int);
void rehash_table(stringtable_ref table, size_t new_size);
float update_loadf(stringtable_ref table);
uint32_t nearest_power_two(uint32_t);

stringtable_ref new_stringtable(void) {
    stringtable_ref new_table;
    if ((new_table = (stringtable_ref)malloc(sizeof(struct stringtable))) == NULL) {
        return NULL;
    }
    new_table->node_count = 0;
    new_table->dimension = BASE_DIMENSION;
    new_table->ops_count = 0;
    new_table->loadf = 0.0f;
    new_table->slots_occupied = 0;

    if ((new_table->table_head = (stringnode_ref *)calloc(new_table->dimension, sizeof(stringnode_ref))) == NULL) {
        return NULL;
    }

    return new_table;
}

inline uint32_t nearest_power_two(uint32_t num) {
    uint32_t n = num > 0 ? num - 1 : 0;

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    DEBUGF('a', "POW2: %d", n);
    return n;
}

void debugdump_stringtable(stringtable_ref table, FILE* log) {
    if(!table)
        return;
    uint32_t i, dim = table->dimension;
    stringnode_ref *head = table->table_head;
    
    DEBUGF('a', "TABLE\n D: %12d AD: %12d\nLF: %12.6f OC: %12u\nHead PTR: %p\n",
        table->dimension, table->node_count, 
        table->loadf, table->ops_count, table->table_head);

    for(i = 0; i < dim; i++) {
        debugdump_node_chain(head[i], log, i);
    }

}

void debugdump_node_chain(stringnode_ref chain_head, FILE * log, unsigned int table_offset) {
    if(!chain_head)
        return;
    stringnode_ref mover = chain_head;

    DEBUGF('d', "< %16p >\n\t< %16p >\n\t< %16p >\n==>  ", mover, mover->next, mover->prev);


    fprintf(log, "%8d  %12u  \"%s\"\n", table_offset, chain_head->node_hash, chain_head->node_string);
    mover = mover->next;
    while(mover != NULL) {
        fprintf(log, "          %12u  \"%s\"\n", mover->node_hash, mover->node_string);
        mover = mover->next;
    }

}

stringnode_ref intern_stringtable(stringtable_ref table, cstring string) {
    if(!table)
        return NULL;

    uint32_t dimension = table->dimension;
    stringnode_ref *head = table->table_head;
    hashcode_t string_hash = strhash(string);
    stringnode_ref intern_node = find_node(head, dimension, string_hash);

    DEBUGF('i', "HASH: %d\t FOUND_NODE: %p\n", string_hash, intern_node);
    if(!intern_node) {
        intern_node = (stringnode_ref)malloc(sizeof(struct stringnode));
        intern_node->node_hash = string_hash;

        intern_node->node_string = (char *)malloc(strlen(string) + 1);
        strncpy(intern_node->node_string, string, strlen(string));

        intern_node->prev = intern_node->next = NULL;
        table->node_count++;
        table->ops_count++;

        if(head[string_hash % dimension] == NULL) {
            head[string_hash % dimension] = intern_node;
            table->slots_occupied++;
        } else {
            stringnode_ref chain_tail = find_chain_tail(head[string_hash % dimension]);
            chain_tail->next = intern_node;
            intern_node->prev = chain_tail;
        }
    }
    if(!(table->ops_count % REHASH_OPS_THRESHOLD)) {
        float nloadf = update_loadf(table);
        if(nloadf >= LOAD_MAX) {
            rehash_table(table, nearest_power_two(table->node_count * 2) - 1);
        }
    }
    return intern_node;
}

cstring peek_stringtable(stringnode_ref node) {
    return node->node_string;
}
cstring repr_stringnode(stringnode_ref node) {
    cstring repr_string = (cstring)calloc(strlen(node->node_string) + 14, sizeof(char));
    sprintf(repr_string, "%s %12d", node->node_string, node->node_hash);
    return repr_string;
}
hashcode_t hashcode_stringtable(stringnode_ref node) {
    return node->node_hash;
}

void rehash_table(stringtable_ref table, size_t new_size) {
    stringnode_ref *new_table = (stringnode_ref *)calloc(new_size, sizeof(stringnode_ref));
    unsigned int i;
    table->slots_occupied = 0;

    DEBUGF('a', "REHASH! OLD SIZE: %d NEW SIZE: %d\n", table->dimension, new_size);
    for(i = 0; i < table->dimension; i++) {
        stringnode_ref mover = table->table_head[i];
        if(!mover) continue;
        stringnode_ref next = mover->next;
        for(; mover != NULL; mover = next, next = mover == NULL ? NULL : mover->next) {
            mover->prev = NULL;
            mover->next = NULL;

            uint32_t new_offset = mover->node_hash % new_size;
            DEBUGF('a', "NODE: %p OLD_LOC %d NEW_LOC %d\n", mover, mover->node_hash % table->dimension, new_offset);
            
            if(new_table[new_offset] == NULL) {
                DEBUGF('a', "NEW SLOT: %d OCC_SLOT %d\n", new_offset, table->slots_occupied + 1);
                new_table[new_offset] = mover;
                table->slots_occupied++;                
            } else {
                stringnode_ref chain_tail = find_chain_tail(new_table[new_offset]);
                DEBUGF('a', "CHAINED_NODE TAIL: %p\n", chain_tail);
                chain_tail->next = mover;
                mover->prev = chain_tail;
            }
        }   
    }
    free(table->table_head);
    table->table_head = new_table;
    table->dimension = new_size;
    update_loadf(table);
}

float update_loadf(stringtable_ref table) {
    float new_loadf = ( (table->node_count / (LOAD_MAX * COLLISION_CHAIN_MAX * table->slots_occupied ) ) + 
                        (table->slots_occupied / table->dimension) );
    DEBUGF('a', "LOADF %f\n", new_loadf);
    table->loadf = new_loadf;
    return new_loadf;
}

stringnode_ref find_chain_tail(stringnode_ref chain) {
    if(!chain)
        return NULL;
    stringnode_ref mover, tail;
    for(tail = chain, mover = chain->next; mover != NULL; tail = mover, mover = mover->next);
    return tail;
}

stringnode_ref find_node(stringnode_ref *head, uint32_t dimension, hashcode_t hash) {
    return find_node_from_chain(head[hash % dimension], hash);
}

stringnode_ref find_node_from_chain(stringnode_ref chain, hashcode_t hash) {
    if(!chain)
        return NULL;
    stringnode_ref mover = chain;
    while(mover != NULL) {
        if(mover->node_hash == hash)
            return mover;
        mover = mover->next;
    }
    return NULL;
}

void delete_stringtable(stringtable_ref table) {
    if(!table)
        return;

    stringnode_ref *mover = table->table_head;
    unsigned int i = 0;
    while(i++ < table->dimension) {
        delete_node_chain(*mover);
        mover++;
    }
    free(table->table_head);
    free(table);
}

//Deletes all nodes chained off the specified node
void delete_node_chain(stringnode_ref node) {
    stringnode_ref mover, tail;
    if(!node)
        return;

    for(mover = node->next, tail = node; mover != NULL; tail = mover, mover = mover->next) {
        free(tail->node_string);
        free(tail);
    }
    free(tail->node_string);
    free(tail);
}

//Deletes a specified node, removing itself from the chain linked list
void delete_node(stringnode_ref node) {
    stringnode_ref node_prev, node_next;
    
    if(!node)
        return;

    node_prev = node->prev;
    node_next = node->next;
    if(node_prev) {
        node_prev->next = node_next;
    }
    if(node_next) {
        node_next->prev = node_prev;
    }

    free(node->node_string);
    free(node);
}
