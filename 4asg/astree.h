// Payton Schwarz, peschwar@ucsc.edu
// Alexus Munn, ammunn@ucsc.edu

#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
#include <bitset>
#include <unordered_map>
using namespace std;
#include "auxlib.h"
struct symbol;
using symbol_table = unordered_map<string*,symbol*>;

enum { ATTR_void, ATTR_int, ATTR_null, ATTR_string,
       ATTR_struct, ATTR_array, ATTR_function,
       ATTR_variable, ATTR_field, ATTR_typeid,
       ATTR_param, ATTR_lval, ATTR_const, ATTR_vreg,
       ATTR_vaddr, ATTR_bitset_size,
};

using attr_bitset = bitset<ATTR_bitset_size>;


struct location {
   size_t filenr;
   size_t linenr;
   size_t offset;
};

struct astree {

   // Fields.
   int symbol;                  // token code
   location lloc;               // source location
   const string* lexinfo;       // pointer to lexical information
   vector<astree*> children;    // children of this n-way node
   attr_bitset attributes;      // Indicates AST props for code gen
   size_t block_nr;             // Block# to which this symbol belongs
   symbol_table* struct_table;
   // The following is coordinates for the ident declaration
   //location idecl_lloc;
    size_t id_filenr;
    size_t id_linenr;
    size_t id_offset;


   // Functions.
   astree (int symbol, const location&, const char* lexinfo);
   ~astree();
   astree* adopt (astree* child1, astree* child2 = nullptr, astree* child3 = nullptr);
   astree* adopt_sym (astree* child, int symbol);
   // Funcs I've added.
   astree* change_sym (int symbol_);
   // End func I've added
   void dump_node (FILE*);
   void dump_tree (FILE*, int depth = 0);
   static void dump (FILE* outfile, astree* tree);
   static void print (FILE* outfile, astree* tree, int depth = 0);
};

astree* adopt_func (astree* ident, astree* paramlist, astree* block);

void destroy (astree* tree1, astree* tree2 = nullptr, astree* tree3 = nullptr);

void errllocprintf (const location&, const char* format, const char*);

string enum_attr (attr_bitset ab);

#endif

