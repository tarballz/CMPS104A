#include "symbol_stack.h"
#include <iostream>
#include "astree.h"

size_t next_block = 0;

void symbol_stack::enter_block() {
    ++next_block;
    symbol_stack.push_back(nullptr);
    blocknr_stack.push_back(next_block);
}

void symbol_stack::leave_block() {
    symbol_stack.pop_back();
    blocknr_stack.pop_back();
}

void symbol_stack::define_ident(astree *node) {
    if(symbol_stack.back() == nullptr) {
        symbol_stack.back() = new symbol_table;
    } else {
        insert_symtab(symbol_stack.back(), node);
    }
}

symbol* symbol_stack::search_ident(astree* node) {
    for(symbol_table* i : symbol_stack) {
        if(i == nullptr || i->empty()) {
            continue;
        }
        symbol* s = lookup(i, node);
        if(s != nullptr) {
            return s;
        }
    }
    return nullptr;
}