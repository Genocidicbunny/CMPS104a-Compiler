#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <stdbool.h>

#include "auxlib.h"
// #include "stringtable.h"

typedef struct astree_rep *astree;

bool is_astree (void *object);
astree new_astree (int symbol, int filenr, int linenr, int offset,
                   char* lexinfo);
astree adopt (astree root, /*ASTree*/ ... /*, NULL */);
astree adopt1 (astree root, astree child);
astree adopt2 (astree root, astree left, astree right);
astree adopt1sym (astree root, astree child, int symbol);
void dump_astree (FILE *outfile, astree root);
void yyprint (FILE *outfile, unsigned short toknum, astree yyvaluep);
void freeast (astree tree);

#define freeast2(T1,T2) { freeast (T1); freeast (T2); }

// LINTED(static unused)
RCSH(ASTREE_H,"$Id: astree.h,v 1.7 2012-10-11 18:22:20-07 - - $")
#endif
