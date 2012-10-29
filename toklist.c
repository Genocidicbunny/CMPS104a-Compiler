#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "toklist.h"

toklist_ref new_toklist();
toknode_ref head_toklist(toklist_ref);
toknode_ref tail_toklist(toklist_ref);
toknode_ref add_toknode(toklist_ref, astree, char *);
void delete_toklist(toklist_ref);
void delete_toknode(toknode_ref);
void delete_toknode_destructive(toknode_ref);
void dump_toklist(toklist_ref, FILE*);


toklist_ref 
new_toklist() {
    toklist_ref toklist = (toklist_ref)malloc(sizeof(struct toklist));
    toknode_ref toknode = (toknode_ref)malloc(sizeof(struct toknode));
    toknode->token_node = NULL;
    toknode->token_string = NULL;
    toknode->prev = toknode->next = NULL;
    toklist->head = toklist->tail = toknode;
    return toklist;
}

toknode_ref 
head_toklist(toklist_ref list) {
    if(list->head == list->tail && list->head->token_string == NULL) 
        return NULL;
    else
        return list->head;
}

toknode_ref 
tail_toklist(toklist_ref list) {
    if(list->head == list->tail && list->head->token_string == NULL)
        return NULL;
    else
        return list->tail;
}

toknode_ref 
add_toknode(toklist_ref list, astree astnode, char* tok_str) {
    toknode_ref new_node = (toknode_ref)malloc(sizeof(struct toknode));
    new_node->token_node = astnode;
    new_node->token_string = strdup(tok_str);
    new_node->next = NULL;
    new_node->prev = list->tail;
    
    list->tail->next = new_node;
    list->tail = new_node;
    
    return new_node;
}

void 
delete_toklist(toklist_ref list) {
    toknode_ref mover, next;
    
    for(mover = list->head, next = list->head->next; next != NULL;
        mover = next, next = next->next) {
        delete_toknode(mover);
    }
    delete_toknode(mover);
}

void 
delete_toknode(toknode_ref node) {
    node->token_node = NULL;
    delete_toknode_destructive(node);
}

void 
delete_toknode_destructive(toknode_ref node) {
    if(node->token_node != NULL) freeast(node->token_node);
    if(node->token_string != NULL) free(node->token_string);
    free(node);
}

void 
dump_toklist(toklist_ref list, FILE * output) {
    toknode_ref mover;
    
    for(mover = list->head->next; mover != NULL; mover = mover->next) {
        fprintf(output, "%s", mover->token_string);
        DEBUGF('k', "%s\n", mover->token_string);
    }
}
