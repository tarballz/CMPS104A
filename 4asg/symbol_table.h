#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#include <vector>
#include <unordered_map>
#include <bitset>
#include "astree.h"

struct symbol;
using symbol_table = unordered_map<string*, symbol*>;
// NOTE: symbol_entry = pair<const string*, symbol*>
// where key = pointer into strset as returned by
// the intern function
// typedef pair<const Key, T> value_type
using symbol_entry = symbol_table::value_type;

struct symbol {
  attr_bitset attributes;        // Indicates AST props for code gen
  symbol_table* fields;          // Pointer to 'fields' table if
                                 // sym is a struct, null otherwise
  location lloc;
  size_t block_nr;                // Block# to which this symbol belongs
  vector<symbol*>* parameters;   // Points at the param_list, points
                                 // at vec for a func, otherwise null
  // The following is coordinates for the ident declaration
  //location idecl_lloc;
  size_t id_filenr;
  size_t id_linenr;
  size_t id_offset;
};

symbol* create_symbol(astree* node);
void insert_symtab(symbol_table* symtab, astree* node);
symbol* lookup (symbol_table* symtab, astree* node);

#endif
