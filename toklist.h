#ifndef __TOKLIST_H__
#define __TOKLIST_H__

#include "astree.h"

typedef struct toknode * toknode_ref;
typedef struct toklist * toklist_ref;

struct toknode {
    astree token_node;
    char *token_string;
    toknode_ref prev;
    toknode_ref next;
};

struct toklist {
    toknode_ref head;
    toknode_ref tail;
};

toklist_ref token_list;

toklist_ref new_toklist();
toknode_ref head_toklist(toklist_ref);
toknode_ref tail_toklist(toklist_ref);
toknode_ref add_toknode(toklist_ref, astree, char *);
void delete_toklist(toklist_ref);
void dump_toklist(toklist_ref, FILE*);


#endif
