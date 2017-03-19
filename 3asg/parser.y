%{
// Payton Schwarz, peschwar@ucsc.edu
// Alexus Munn, ammunn@ucsc.edu

#include <cassert>

#include "lyutils.h"
#include "astree.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%initial-action {
  parser::root = new astree (TOK_ROOT, {0,0,0}, "<<ROOT>>");
}


%token TOK_VOID TOK_CHAR TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_NULL TOK_NEW TOK_ARRAY
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_ORD TOK_CHR TOK_ROOT

%token TOK_RETURNVOID TOK_PARAMLIST TOK_PROTOTYPE TOK_DECLID
%token TOK_NEWSTRING TOK_VARDECL TOK_INDEX TOK_FUNCTION

%right TOK_IFELSE TOK_IF TOK_ELSE
%right '=' 
%left TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left '+' '-'
%left '*' '/' '%'
%right TOK_POS TOK_NEG '!' TOK_NEW
%left TOK_ARRAY TOK_FIELD TOK_FUNCTION '.' '['
%nonassoc TOK_PARENS

%start start

%%

start   : program             { parser::root = $1; }
        ;

program  : program structdef    { $$ = $1->adopt($2); }
         | program function     { $$ = $1->adopt($2); }
         | program statement    { $$ = $1->adopt($2); }
         | program error '}'    { $$ = $1; }
         | program error ';'    { $$ = $1; }
         |                      { $$ = parser::root; }

structdef : TOK_STRUCT TOK_IDENT '{' fielddecl_list '}'  
            { destroy($3, $5); 
              $$ = $1->adopt_sym($2, TOK_TYPEID)->adopt($3); }
          | TOK_STRUCT TOK_IDENT '{' '}'                 
            { destroy($3, $4); $$ = $1->adopt_sym($2, TOK_TYPEID); }
          ;

fielddecl_list: fielddecl_list fielddecl ';'              
                  { destroy($3); $$ = $1->adopt($2); }
              | fielddecl ';'                             
                  { destroy($2); $$ = $1; }
              ;

fielddecl: basetype TOK_IDENT        
           { $$ = $1->adopt_sym($2, TOK_FIELD); }
         | basetype TOK_ARRAY TOK_IDENT                  
           { $$ = $2->adopt($1)->adopt_sym($3, TOK_FIELD); }
         ;

basetype: TOK_VOID    { $$ = $1; }
        | TOK_INT     { $$ = $1; }
        | TOK_STRING  { $$ = $1; }
        | TOK_CHAR    { $$ = $1; }
        | TOK_IDENT   { $$ = $1; }
        | TOK_TYPEID  { $$ = $1; }
        ;

function: identdecl param_list ')' block              
          { destroy($3); $$ = $1->adopt_func($2, $4); }


param_list: param_list ',' identdecl              
                    { $$ = $1->adopt($3); }
           | '(' identdecl                            
                    { $$ = $1->adopt_sym($2, TOK_PARAMLIST); }
           | '('                                      
                    { $$ = $1->change_sym(TOK_PARAMLIST); }
           ;

identdecl: basetype TOK_ARRAY TOK_IDENT        
           { $1 = $1->change_sym(TOK_DECLID); $$=$2->adopt($1, $3); }
         | basetype TOK_IDENT                  
           { $$ = $1->adopt_sym($2, TOK_DECLID); }
         ;

block    : blockhead '}'       
           { destroy($2); $1=$1->change_sym(TOK_BLOCK); $$=$1; }
         | '{' '}'             
           { destroy($2); $1=$1->change_sym(TOK_BLOCK); $$=$1; }
         | ';'                 
           { $1=$1->change_sym(TOK_BLOCK); $$=$1; }
         ;

blockhead: blockhead statement                 { $$ = $1->adopt($2); }
         | '{' statement                       { $$ = $1->adopt($2); }
         ;

statement     : block                          { $$ = $1; }
              | vardecl                        { $$ = $1; }
              | while                          { $$ = $1; }
              | ifelse                         { $$ = $1; }
              | return                         { $$ = $1; }
              | expr ';'                       { destroy($2); $$ = $1; }
              ;

vardecl: identdecl '=' expr ';'    
            { destroy($4); $2 = $2->change_sym(TOK_VARDECL); 
              $$ = $2->adopt($1, $3);}
       ;

while: whilehead statement                     {$$ = $1->adopt($2);}
     ;

whilehead: TOK_WHILE '(' expr ')'              
           {destroy($2, $4); $$ = $1->adopt($3);}
         ;

ifelse: TOK_IF '(' expr ')' statement %prec TOK_ELSE         
               {destroy($2, $4); $$ = $1->adopt($3, $5);}
      | TOK_IF '(' expr ')' statement TOK_ELSE statement   
                {$1 = $1->adopt_sym($3, TOK_IFELSE); 
                 $$ = $1->adopt($5, $7); destroy($2, $4);
                 destroy($6);}
      ;

return: TOK_RETURN expr ';'            
         { $$ = $1->adopt($2); }
      | TOK_RETURN ';'                     
         { $$ = $1->adopt_sym($2, TOK_RETURNVOID); }
      ;

expr: BINOP                    { $$ = $1; }
    | UNOP                     { $$ = $1; }
    | allocator %prec TOK_NEW  { $$ = $1; }
    | call                     { $$ = $1; }
    | '(' expr ')'             { destroy($1, $3); $$ = $2; }
    | variable                 { $$ = $1; }
    | constant                 { $$ = $1; }
    ;

BINOP: expr TOK_IFELSE expr  { $$ = $2->adopt($1, $3); }
    | expr TOK_EQ expr       { $$ = $2->adopt($1, $3); }
    | expr TOK_NE expr       { $$ = $2->adopt($1, $3); }
    | expr TOK_LT expr       { $$ = $2->adopt($1, $3); }
    | expr TOK_LE expr       { $$ = $2->adopt($1, $3); }
    | expr TOK_GT expr       { $$ = $2->adopt($1, $3); }
    | expr TOK_GE expr       { $$ = $2->adopt($1, $3); }
    | expr '+' expr          { $$ = $2->adopt($1, $3); }
    | expr '-' expr          { $$ = $2->adopt($1, $3); }
    | expr '*' expr          { $$ = $2->adopt($1, $3); }
    | expr '/' expr          { $$ = $2->adopt($1, $3); }
    | expr '%' expr          { $$ = $2->adopt($1, $3); }
    | expr '=' expr          { $$ = $2->adopt($1, $3); }
    ;

UNOP: '!' expr                     { $$ = $1->adopt($2); }
    | '-' expr %prec TOK_NEG       { $$ = $1->adopt_sym($2, TOK_NEG); }
    | '+' expr %prec TOK_POS       { $$ = $1->adopt_sym($2, TOK_POS); }
    ;

allocator: TOK_NEW TOK_IDENT '(' ')'                 
            { destroy($3, $4); $$ = $1->adopt_sym($2, TOK_TYPEID); }
         | TOK_NEW TOK_STRING '(' expr ')'           
            { destroy($3, $5); 
              destroy($2); 
              $$ = $1->adopt_sym($4, TOK_NEWSTRING ); }
         | TOK_NEW basetype '[' expr ']'      
            { destroy($3, $5); 
              $$ = $1->adopt_sym($2, TOK_NEWARRAY)->adopt_sym($4, TOK_NEWARRAY);}
         ; 

call     : TOK_IDENT '(' ')'              
            { destroy($3); $$ = $1->adopt_sym($2, TOK_CALL); }
         | TOK_IDENT callargs ')'         
            { destroy($3); $$ = $2->adopt_sym($1, TOK_CALL); }
         ;

callargs : '(' expr                { $$ = $1->adopt($2); }
         | callargs ',' expr       { destroy($2); $$ = $1->adopt($3); }
         ;

variable : TOK_IDENT                     { $$ = $1; }
         | expr '[' expr ']'             
            { $2=$2->change_sym(TOK_INDEX); $$ = $2->adopt($1, $3);}
         | expr '.' expr                 
            { $3=$3->change_sym(TOK_FIELD); $$ = $2->adopt($1, $3);}
         ;

constant : TOK_INTCON     { $$ = $1; }
          | TOK_CHARCON   { $$ = $1; }
          | TOK_STRINGCON { $$ = $1; }
          | TOK_NULL      { $$ = $1; }
         ;

%%


const char *parser::get_tname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}

bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

/*
static void* yycalloc (size_t size) {
   void* result = calloc (1, size);
   assert (result != nullptr);
   return result;
}
*/

