#include "symbol_table.h"
#include <iostream>
#include "astree.h"

symbol* create_symbol(astree* node) {
    symbol* s = new symbol();
    s->lloc = node->lloc;
    s->attributes = node->attributes;
    s->block_nr = node->block_nr;
    s->parameters = nullptr;

    return s;
}

void insert_symtab (symbol_table* symtab, astree* node) {
    symbol* s = create_symbol(node);
    string* no_const_lex = const_cast<string*>(node->lexinfo);
    symbol_entry se = symbol_entry(no_const_lex, s);
    symtab->insert(se);
}

symbol* lookup (symbol_table* symtab, astree* node) {
    string* no_const_lex = const_cast<string*>(node->lexinfo);
    if (symtab->count(no_const_lex) <= 0) {
        return nullptr;
    }
    symbol_entry se = *symtab->find(no_const_lex);
    node->id_filenr = se.second->lloc.filenr;
    node->id_linenr = se.second->lloc.linenr;
    node->id_offset = se.second->lloc.offset;
    return se.second;
}

