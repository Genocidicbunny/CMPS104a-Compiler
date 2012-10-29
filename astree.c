
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auxlib.h"
#include "astree.h"
#include "astree.rep.h"
#include "lyutils.h"
// #include "stringtable.h"

static char *astree_tag = "struct astree_rep";
extern stringtable_ref table;

bool is_astree (void *object) {
   astree tree = (astree) object;
   return tree != NULL && tree->tag == astree_tag;
}

astree new_astree (int symbol, int filenr, int linenr, int offset,
                   char* lexinfo) {
   size_t size = sizeof (struct astree_rep);
   astree tree = malloc (size);
   assert (tree != NULL);
   tree->tag = astree_tag;
   tree->symbol = symbol;
   tree->filenr = filenr;
   tree->linenr = linenr;
   tree->offset = offset;
   tree->lexinfo = intern_stringtable(table, lexinfo);
   // tree->lexinfo = strdup (lexinfo);
   assert (tree->lexinfo != NULL);
   tree->first = NULL;
   tree->last = NULL;
   tree->next = NULL;
   DEBUGF ('f', "malloc (%d) = %p-> %d:%d.%d: %s: %p->\"%s\"\n",
           size, tree, tree->filenr, tree->linenr, tree->offset,
           get_yytname (tree->symbol), tree->lexinfo, tree->lexinfo->node_string);
   return tree;
}

astree adopt (astree root, ...) {
   va_list children;
   assert (is_astree (root));
   va_start (children, root);
   for(;;) {
      astree child = va_arg (children, astree);
      if (child == NULL) break;
      assert (is_astree (child));
      if (root->last == NULL) root->first = child;
                         else root->last->next = child;
      root->last = child;
      DEBUGF ('a', "%p (%s) adopting %p (%s)\n",
              root, root->lexinfo->node_string,
              child, child->lexinfo->node_string);
   }
   va_end (children);
   return root;
}

astree adopt2 (astree root, astree left, astree right) {
   return adopt (root, left, right, NULL);
}

astree adopt1 (astree root, astree child) {
   return adopt (root, child, NULL);
}

astree adopt1sym (astree root, astree child, int symbol) {
   root = adopt1 (root, child);
   root->symbol = symbol;
   return root;
}

static void dump_node (FILE *outfile, astree node, int depth) {
   assert (is_astree (node));
   fprintf (outfile, "%p-> astree {%s(%d), %d:%d.%03d, %p->\"%s\",\n",
             (void*) node, get_yytname (node->symbol), node->symbol,
             node->filenr, node->linenr, node->offset,
             node->lexinfo, (node->lexinfo)->node_string);
   fprintf (outfile, "%*sfirst=%p, last=%p, next=%p}",
             depth * 3 + 12, "", (void*) node->first,
             (void*) node->last, (void*) node->next);
}

static void dump_astree_rec (FILE *outfile, astree root, int depth) {
   astree child = NULL;
   if (root == NULL) return;
   assert (is_astree (root));
   fprintf (outfile, "%*s%s ", depth * 3, "", (root->lexinfo)->node_string);
   dump_node (outfile, root, depth);
   fprintf (outfile, "\n");
   for (child = root->first; child != NULL; child = child->next) {
      dump_astree_rec (outfile, child, depth + 1);
   }
}

void dump_astree (FILE *outfile, astree root) {
   dump_astree_rec (outfile, root, 0);
   fflush (NULL);
}

void yyprint (FILE *outfile, unsigned short toknum, astree yyvaluep) {
   fprintf (outfile, "%d=%s)\n%*s(",
             toknum, get_yytname (toknum), 9, "");
   if (is_astree (yyvaluep)) {
      dump_node (outfile, yyvaluep, 3);
   }else{
      fprintf (outfile, "yyvaluep = %p", (void*) yyvaluep);
   }
   fflush (NULL);
}

void freeast (astree root) {
   astree child = NULL;
   if (root == NULL) return;
   assert (is_astree (root));
   for (child = root->first; child != NULL;) {
      astree asttofree = child;
      assert (is_astree (asttofree));
      child = child->next;
      freeast (asttofree);
   }
   DEBUGF ('f', "free [%X]-> %d:%d.%d: %s: %p->\"%s %d\")\n",
           (uintptr_t) root, root->filenr, root->linenr, root->offset,
            get_yytname (root->symbol), root->lexinfo, root->lexinfo->node_string, root->lexinfo->node_hash);
   //free (root->lexinfo);
   memset (root, 0, sizeof (struct astree_rep));
   free (root);
}

// LINTED(static unused)
RCSC(ASTREE_C,"$Id: astree.c,v 1.11 2012-10-11 18:22:20-07 - - $")

