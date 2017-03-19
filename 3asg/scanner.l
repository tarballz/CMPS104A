/* Payton Schwarz, peschwar@ucsc.edu */
/* Alexus Munn, ammunn@ucsc.edu */
%{

#include "auxlib.h"
#include "lyutils.h"
extern FILE* tok_out;

#define YY_USER_ACTION  { lexer::advance(); }

#define RETURN(SYMBOL) { \
    yylval = new astree (SYMBOL, lexer::lloc, yytext); \
    fprintf(tok_out, "%lu  %lu.%03lu   %3d %-15s (%s)\n",\
            yylval->lloc.filenr,yylval->lloc.linenr, \
            yylval->lloc.offset, \
            SYMBOL,parser::get_tname(SYMBOL),yytext); \
    return SYMBOL; \
}

%}

%option 8bit
%option debug
%option nodefault
%option noinput
%option nounput
%option noyywrap
%option warn
/*%option verbose*/

LETTER          [A-Za-z_]
DIGIT           [0-9]
NUMBER          {DIGIT}+
CHAR            ('([^\\'\n]|\\[\\'"0nt])')
STRING          (\"([^\\"\n]|\\[\\'"0nt])*\")
IDENT           ({LETTER}({LETTER}|{DIGIT})*)
BADCHAR         ('([^\\'\n]|\\[\\'"0nt])|('\\([^\\'"0tn])'))
BADSTR          (\"([^\\"\n]|\\[\\'"0nt])*\)|('\\([^\\'"0tn])')*)
BADIDENT        [0-9]+[a-zA-Z_]+

%%

"#".*           { lexer::include(); fprintf(tok_out, "%s\n", yytext); }
[ \t]+          { }
\n              { lexer::newline();   }

"char"          { RETURN (TOK_CHAR);  }
"else"          { RETURN (TOK_ELSE);  }
"if"            { RETURN (TOK_IF);    }
"int"           { RETURN (TOK_INT);   }
"new"           { RETURN (TOK_NEW);   }
"null"          { RETURN (TOK_NULL);  }
"return"        { RETURN (TOK_RETURN);}
"string"        { RETURN (TOK_STRING);}
"struct"        { RETURN (TOK_STRUCT);}
"void"          { RETURN (TOK_VOID);  }
"while"         { RETURN (TOK_WHILE); }
"=="            { RETURN (TOK_EQ);    }
"!="            { RETURN (TOK_NE);    }
"<"             { RETURN (TOK_LT);    }
"<="            { RETURN (TOK_LE);    }
">"             { RETURN (TOK_GT);    }
">="            { RETURN (TOK_GE);    }
"[]"            { RETURN (TOK_ARRAY); }

{NUMBER}        { RETURN (TOK_INTCON); }
{IDENT}         { RETURN (TOK_IDENT); }
"="             { RETURN ('='); }
"+"             { RETURN ('+'); }
"-"             { RETURN ('-'); }
"*"             { RETURN ('*'); }
"/"             { RETURN ('/'); }
"^"             { RETURN ('^'); }
"("             { RETURN ('('); }
")"             { RETURN (')'); }
"{"             { RETURN ('{'); }
"}"             { RETURN ('}'); }
","             { RETURN (','); }
"."             { RETURN ('.'); }
"!"             { RETURN ('!'); }
"%"             { return ('%'); }
";"             { RETURN (';'); }
{BADCHAR}|{BADSTR}|{BADIDENT} { lexer::badtoken (yytext); }
.               { lexer::badchar (*yytext); }

%%
