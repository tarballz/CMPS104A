#include <string>
#include <iostream>
#include "typecheck.h"

astree* end_struct_node = nullptr;

void print_symbol_table(FILE* ofile, astree* node, symbol_stack* ss, symbol_table* struct_table) {
    if(node->block_nr == 0 && !node->attributes[ATTR_field]) {
        fprintf(ofile, "\n");
    } else if(node->attributes[ATTR_struct]==1) {
        fprintf(ofile, "\n");
    } else {
        fprintf(ofile, "  ");
    }
    if(!node->attributes[ATTR_field]) {
        fprintf(ofile,
                "%s (%zd.%zd.%zd) {%zd} %s",
                node->lexinfo->c_str(),
                node->lloc.filenr,
                node->lloc.linenr,
                node->lloc.offset,
                node->block_nr,
                enum_attr(node->attributes).c_str()
        );
    } else {
        fprintf(ofile,
                "%s (%zd.%zd.%zd) {%s} %s",
                node->lexinfo->c_str(),
                node->lloc.filenr,
                node->lloc.linenr,
                node->lloc.offset,
                end_struct_node->lexinfo->c_str(),
                enum_attr(node->attributes).c_str()
        );
    }
    if(node->attributes[ATTR_struct]==1) {
        fprintf(ofile,
                "\"%s\"",
                node->lexinfo->c_str()
        );
        end_struct_node = node;
    }
    fprintf(ofile, "\n");
}

bool is_prim_type(astree* node) {
    if(node->attributes[ATTR_int]==1) {
        return true;
    }
    return false;
}

bool is_ref_type(astree* node) {
    if(node->attributes[ATTR_null]==1) {
        return true;
    } else if (node->attributes[ATTR_string]==1) {
        return true;
    } else if (node->attributes[ATTR_struct]==1) {
        return true;
    } else if (node->attributes[ATTR_array]==1) {
        return true;
    }
    return false;
}

bool is_prim_or_ref (astree* node) {
    if(is_prim_type(node)){
        return true;
    } else if (is_ref_type(node)) {
        return true;
    }
    return false;
}

bool is_compat(astree* node1, astree* node2) {
    if(is_prim_type(node1) && is_prim_type(node2)){return true;}
    if(is_ref_type(node1) && is_ref_type(node2)){return true;}
    if(is_prim_or_ref(node1) && node2->attributes[ATTR_null]){return true;}
    if(node1->attributes[ATTR_null] && is_prim_or_ref(node2)){return true;}
    return false;
}

void synth_type(astree* parent, astree* child) {
    for(int i = 0; i < ATTR_function; ++i) {
        if(child->attributes[i] == 1) {
            parent->attributes[i] = 1;
        }
    }
}

void synth_attrs(astree* parent, astree* child) {
    for(int i = 0; i < ATTR_bitset_size; ++i) {
        if(child->attributes[i] == 1) {
            parent->attributes[i] = 1;
        }
    }
}
void block_dfs(astree* node, symbol_stack* ss) {
    if(node->symbol == TOK_BLOCK) {
        ss->enter_block();
    }
    node->block_nr = ss->blocknr_stack.back();
    for (auto child : node->children) {
        block_dfs(child, ss);
    }
}

void func_dfs(FILE* ofile, astree* node, symbol_table* struct_table, symbol_stack* ss) {
    node->children[0]->children[0]->attributes[ATTR_function] = 1;
    insert_symtab(ss->symbol_stack[0], node->children[0]->children[0]);
    print_symbol_table(ofile,node->children[0]->children[0], ss, struct_table);
    //ss->enter_block();
    for(auto parameter : node->children[1]->children) {
        parameter->children[0]->attributes[ATTR_variable] = 1;
        parameter->children[0]->attributes[ATTR_lval] = 1;
        parameter->children[0]->attributes[ATTR_param] = 1;
        parameter->children[0]->block_nr = next_block;
        ss->define_ident(parameter->children[0]);
        print_symbol_table(ofile, parameter->children[0],  ss, struct_table);
    }
    block_dfs(node->children[2], ss);
    ss->leave_block();
}

void proto_dfs(FILE* ofile, astree* node , symbol_table* struct_table, symbol_stack* ss) {
    node->children[0]->children[0]->attributes[ATTR_function] = 1;
    insert_symtab(ss->symbol_stack[0], node->children[0]->children[0]);
    print_symbol_table(ofile, node->children[0]->children[0], ss, struct_table);
    ss->enter_block();
    for(auto param : node->children[1]->children){
        param->children[0]->attributes[ATTR_variable] = 1;
        param->children[0]->attributes[ATTR_lval] = 1;
        param->children[0]->attributes[ATTR_param] = 1;
        param->children[0]->block_nr = next_block;
        ss->define_ident(param->children[0]);
        print_symbol_table(ofile, param->children[0], ss, struct_table);
    }
    ss->leave_block();
}


void typecheck_assign(FILE* ofile, astree* node, symbol_stack* ss, symbol_table* struct_table) {
    astree* left_child = nullptr;
    astree* right_child = nullptr;
    symbol* sym;
    if(node->children.size() >= 1){
        left_child = node->children[0];
    }
    if(node->children.size() >= 2){
        right_child = node->children[1];
    }
    switch(node->symbol){
        case TOK_ROOT:
        case TOK_DECLID:
        case TOK_WHILE:
        case TOK_IF:
        case TOK_IFELSE:
        case TOK_PARAMLIST:
        case TOK_RETURN:
        case TOK_RETURNVOID:
        case '(':
        case ')':
        case '}':
        case ']':
        case ';':
            break;
        case TOK_INDEX:
            node->attributes[ATTR_lval] = 1;
            node->attributes[ATTR_vaddr] = 1;
            break;
        case TOK_EQ:
        case TOK_NE:
            if(is_compat(left_child, right_child)) {
                synth_type(node, left_child);
                node->attributes[ATTR_vreg] = 1;
            }
            break;
        case TOK_LT:
        case TOK_LE:
        case TOK_GT:
        case TOK_GE:
            {
                if (left_child != nullptr && right_child != nullptr) {
                    if (is_prim_type(left_child) &&
                        is_prim_type(right_child)) {
                        node->attributes[ATTR_int] = 1;
                        node->attributes[ATTR_vreg] = 1;
                    }
                }
                break;
            }
        case '+':
        case '-':
            if(left_child != nullptr && right_child != nullptr) {
                if (left_child->attributes[ATTR_int] &&
                    right_child->attributes[ATTR_int]) {
                    node->attributes[ATTR_vreg] = 1;
                    node->attributes[ATTR_int] = 1;
                }
            }
            break;

        case '*':
        case '/':
        case '%':
            if(left_child != nullptr && right_child != nullptr) {
                if (left_child->attributes[ATTR_int] &&
                    right_child->attributes[ATTR_int]) {
                    node->attributes[ATTR_vreg] = 1;
                    node->attributes[ATTR_int] = 1;
                }
            }
            break;
        case '=':
            if (left_child != nullptr) {
                if (left_child->attributes[ATTR_lval] &&
                    right_child->attributes[ATTR_vreg]) {
                    synth_type(node, left_child);
                    node->attributes[ATTR_vreg] = 1;
                }
            }
            break;
        case TOK_POS:
            if (left_child != nullptr) {
                if (left_child->attributes[ATTR_int]) {
                    node->attributes[ATTR_vreg] = 1;
                    node->attributes[ATTR_int] = 1;
                }
            }
            break;
        case TOK_NEG:
            if(left_child != nullptr) {
                if (left_child->attributes[ATTR_int]) {
                    node->attributes[ATTR_vreg] = 1;
                    node->attributes[ATTR_int] = 1;
                }
            }
            break;
        case '!':
            if (left_child != nullptr) {
                if (left_child->attributes[ATTR_int]) {
                    node->attributes[ATTR_vreg] = 1;
                    node->attributes[ATTR_int] = 1;
                }
            }
            break;
        case TOK_NULL:
            node->attributes[ATTR_null] = 1;
            node->attributes[ATTR_const] = 1;
            break;
        case TOK_CHAR:
            if(left_child != nullptr) {
                left_child->attributes[ATTR_int] = 1;
                node->attributes[ATTR_vreg] = 1;
            }
            break;
        case '.':
            node->attributes[ATTR_vaddr];
            node->attributes[ATTR_lval];
            synth_type(node, left_child);
            break;
        case TOK_NEWSTRING:
            node->attributes[ATTR_string] = 1;
            node->attributes[ATTR_vreg] = 1;
            break;
        case TOK_NEWARRAY:
            node->attributes[ATTR_array] = 1;
            node->attributes[ATTR_vreg] = 1;
            if(left_child->attributes[ATTR_int]){
                node->attributes[ATTR_int] = 1;
            } else if (left_child->attributes[ATTR_string]){
                node->attributes[ATTR_string] = 1;
            } else if (left_child->attributes[ATTR_struct]){
                node->attributes[ATTR_struct] = 1;
            }
            break;
        case TOK_INTCON:
        case TOK_CHARCON:
            node->attributes[ATTR_int]= 1;
            node->attributes[ATTR_const] = 1;
            break;
        case TOK_STRINGCON:
            node->attributes[ATTR_string] = 1;
            node->attributes[ATTR_const] = 1;
            break;
        case TOK_VOID:
            if (left_child != nullptr) {
                left_child->attributes[ATTR_void] = 1;
            }
            break;
        case TOK_IDENT:
            sym = ss->search_ident(node);
            if(sym == nullptr) {
                // Could be a struct.
                sym = lookup(struct_table, node);
            }
            if(sym == nullptr) {
                errprintf("Error %d %d %d: Undefined "
                                  "variable %s\n", node->lloc.filenr, node->lloc.linenr,
                          node->lloc.offset, node->lexinfo->c_str());
                break;
            }
            node->attributes = sym->attributes;
            break;
        case TOK_CALL:
            {
                sym = lookup(ss->symbol_stack[0], node->children.back());
                if (sym == nullptr) {
                    break;
                }
                for (size_t i = 0; i < ATTR_function; ++i) {
                    if (sym->attributes[i]) {
                        node->attributes[i] = 1;
                    }
                }
                break;
            }
        case TOK_FIELD:
            node->attributes[ATTR_field] = 1;
            if (left_child != nullptr) {
                left_child->attributes[ATTR_field] = 1;
                synth_type(node, left_child);
            }
            break;
        case TOK_VARDECL:
            if(left_child != nullptr) {
                left_child->children[0]->attributes[ATTR_lval] = 1;
                left_child->children[0]->attributes[ATTR_variable] = 1;
                synth_attrs(node, left_child);
                if (ss->search_ident(left_child->children[0])) {
                    errprintf("Error %d %d %d: Duplicate declaration %s\n",
                              node->lloc.filenr, node->lloc.linenr, node->lloc.offset,
                              left_child->children[0]->lexinfo->c_str());
                }
                ss->define_ident(left_child->children[0]);
                print_symbol_table(ofile, left_child->children[0], ss, struct_table);
            }
            break;
        case TOK_STRUCT:
            {
                left_child->attributes[ATTR_struct] = 1;
                insert_symtab(struct_table, left_child);
                print_symbol_table(ofile, left_child, ss, struct_table);
                symbol *s = lookup(struct_table, left_child);
                s->fields = new symbol_table;
                for (auto child = node->children.begin() + 1; child != node->children.end(); ++child) {
                    insert_symtab(s->fields, *child);
                    print_symbol_table(ofile, (*child)->children[0], ss, struct_table);
                }
                break;
            }
        case TOK_TYPEID:
            if (left_child == nullptr) {
                break;
            }
            left_child->attributes[ATTR_typeid] = 1;
            break;
        case TOK_INT:
            if (left_child == nullptr) {
                break;
            }
            left_child->attributes[ATTR_int] = 1;
            synth_type(node, left_child);
            break;
        case TOK_STRING:
            if (left_child == nullptr) {
                break;
            }
            left_child->attributes[ATTR_string] = 1;
            synth_type(node, left_child);
            break;
        case TOK_NEW:
            synth_attrs(node, left_child);
            break;
        case TOK_ARRAY:
            if(left_child != nullptr && !left_child->children.empty()) {
                left_child->attributes[ATTR_array] = 1;
                left_child->children[0]->attributes[ATTR_array] = 1;
            }
            break;
        case TOK_BLOCK:
            block_dfs(node, ss);
            ss->leave_block();
            break;
        case TOK_FUNCTION:
            ss->enter_block();
            func_dfs(ofile, node, struct_table, ss);
            print_symbol_table(ofile, node, ss, struct_table);
            break;
        case TOK_PROTOTYPE:
            break;
        default:
            errprintf("Invalid symbol %s\n",
            parser::get_tname(node->symbol));

    }
    if (node->attributes[ATTR_lval] == 1) {
        node->attributes[ATTR_variable] = 1;
    }
}

void typecheck_dfs(FILE* ofile, astree* root, symbol_stack* ss, symbol_table* struct_table, size_t depth){
    for(size_t child = 0; child < root->children.size(); ++child) {
        typecheck_dfs(ofile, root->children[child], ss, struct_table, depth+1);
    }
    typecheck_assign(ofile, root, ss, struct_table);
}

void typecheck(FILE* ofile, astree* root, symbol_stack* ss, symbol_table* struct_table) {
    typecheck_dfs(ofile, root, ss, struct_table, 0);
    while(!ss->symbol_stack.empty()) {
        ss->symbol_stack.pop_back();
    }
}