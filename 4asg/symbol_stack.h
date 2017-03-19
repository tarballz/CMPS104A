#include "symbol_table.h"

extern size_t next_block;

class symbol_stack {
public:
    vector<size_t> blocknr_stack;
    vector<symbol_table*> symbol_stack;
    void enter_block();
    void leave_block();
    void define_ident(astree* node);
    symbol* search_ident(astree* node);
};