#ifndef __STRINGTABLE_H__
#define __STRINGTABLE_H__

#include <stdio.h>
#include <stdint.h>
#include "auxlib.h"

//typedef char * cstring;
//typedef uint32_t hashcode_t;
typedef struct stringtable * stringtable_ref;
typedef struct stringnode * stringnode_ref;

stringtable_ref new_stringtable(void);
void delete_stringtable(stringtable_ref);
void debugdump_stringtable(stringtable_ref, FILE*);
stringnode_ref intern_stringtable(stringtable_ref, cstring);
cstring peek_stringtable(stringnode_ref);
cstring repr_stringnode(stringnode_ref);
hashcode_t hashcode_stringtable(stringnode_ref);


#endif
