// Payton Schwarz, peschwar@ucsc.edu
// Alexus Munn, ammunn@ucsc.edu

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "string_set.h"
#include "lyutils.h"

astree::astree (int symbol_, const location& lloc_, const char* info) {
   symbol = symbol_;
   lloc = lloc_;
   lexinfo = string_set::intern (info);
   // vector defaults to empty -- no children
   block_nr = 0;
   attributes = 0;
   struct_table = nullptr;
}

astree::~astree() {
   while (not children.empty()) {
      astree* child = children.back();
      children.pop_back();
      delete child;
   }
   if (yydebug) {
      fprintf (stderr, "Deleting astree (");
      astree::dump (stderr, this);
      fprintf (stderr, ")\n");
   }
}

astree* astree::adopt (astree* child1, astree* child2, astree* child3) {
   if (child1 != nullptr) children.push_back (child1);
   if (child2 != nullptr) children.push_back (child2);
   if (child3 != nullptr) children.push_back (child3);
   return this;
}

astree* astree::adopt_sym (astree* child, int symbol_) {
   symbol = symbol_;
   return adopt (child);
}

astree* astree::change_sym (int symbol_) {
   this->symbol = symbol_;
   return this;
}

// Trying to get the functions to work...
astree* adopt_func(astree* ident, astree* paramlist, astree* block) {
    int type = TOK_FUNCTION;
    if(!string(";").compare(*block->lexinfo)) {
      type = TOK_PROTOTYPE;
   }
   astree *func = new astree(type, ident->lloc, "");
   func->adopt(ident, paramlist, block);
   return func;
}


void astree::dump_node (FILE* outfile) {
   fprintf (outfile,
            "%p->{%s %zd.%zd.%zd \"%s\":",
            this,
            parser::get_tname (symbol),
            lloc.filenr,
            lloc.linenr,
            lloc.offset,
            lexinfo->c_str()
           );
   for (size_t child = 0; child < children.size(); ++child) {
      fprintf (outfile, " %p", children.at(child));
   }
}

void astree::dump_tree (FILE* outfile, int depth) {
   fprintf (outfile, "%*s", depth * 3, "");
   dump_node (outfile);
   fprintf (outfile, "\n");
   for (astree* child: children) child->dump_tree (outfile, depth + 1);
   fflush (NULL);
}

void astree::dump (FILE* outfile, astree* tree) {
   if (tree == nullptr) fprintf (outfile, "nullptr");
                   else tree->dump_node (outfile);
}

void astree::print (FILE* outfile, astree* tree, int depth) {
   fprintf (outfile, "; %*s", depth * 3, "");

   // Getting rid of TOK_ to make tokens prettier.
   const char *tname = parser::get_tname (tree->symbol);
   if (strstr (tname, "TOK_") == tname) tname += 4;

   fprintf (outfile,
            "%s \"%s\" (%zd.%zd.%zd) {%zd} %s",
            //"%s \"%s\" (%zd.%zd.%zd)\n",
            tname,
            tree->lexinfo->c_str(),
            tree->lloc.filenr,
            tree->lloc.linenr,
            tree->lloc.offset,
            tree->block_nr,
            enum_attr(tree->attributes).c_str()
            );
   if (tree->symbol == TOK_IDENT) {
      fprintf(outfile, "{%zd.%zd.%zd}\n",
              tree->id_filenr,
              tree->id_linenr,
              tree->id_offset
      );
   } else {
      fprintf(outfile, "\n");
   }
   for (astree* child: tree->children) {
      astree::print (outfile, child, depth + 1);
   }
}

void destroy (astree* tree1, astree* tree2, astree* tree3) {
   if (tree1 != nullptr) delete tree1;
   if (tree2 != nullptr) delete tree2;
   if (tree3 != nullptr) delete tree3;
}

void errllocprintf (const location& lloc, const char* format,
                    const char* arg) {
   static char buffer[0x1000];
   assert (sizeof buffer > strlen (format) + strlen (arg));
   snprintf (buffer, sizeof buffer, format, arg);
   errprintf ("%s:%zd.%zd: %s", 
              lexer::filename (lloc.filenr), lloc.linenr, lloc.offset,
              buffer);
}

// This corresponds to the indices of enum in astree.h
static string enum_index (int a) {
    switch (a) {
       case 0: return "void";
       case 1: return "int";
       case 2: return "null";
       case 3: return "string";
       case 4: return "struct";
       case 5: return "array";
       case 6: return "function";
       case 7: return "variable";
       case 8: return "field";
       case 9: return "typeid";
       case 10: return "param";
       case 11: return "lval";
       case 12: return "const";
       case 13: return "vreg";
       case 14: return "vaddr";
       case 15: return "bitset_size";
       default: return "invalid_enum";
    }
}

string enum_attr (attr_bitset ab) {
   string line;
   for (int i = 0; i < ATTR_bitset_size; i++) {
      if (ab[i]) {
         line += (enum_index(i) + " ");
      }
   }
   return line;
}