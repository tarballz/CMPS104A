// Payton Schwarz, peschwar@ucsc.edu
// Alexus Munn, ammunn@ucsc.edu

#include <string>
using namespace std;
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include "lyutils.h"
#include "string_set.h"
#include "auxlib.h"
#include "astree.h"
#include "typecheck.h"

#define YYEOF 0
FILE* tok_out;

const string CPP = "/usr/bin/cpp";
constexpr size_t LINESIZE = 1024;
char* ocfile = NULL;
string_set ss;


// Chomp the last character from a buffer if it is delim.
void chomp(char *string, char delim) {
    size_t len = strlen(string);
    if (len == 0) return;
    char *nlpos = string + len - 1;
    if (*nlpos == delim) *nlpos = '\0';
}


// This should read yyin and fill in the table.
void yy_fill() {
  int tok;
  while((tok = yylex())) {
    if (tok == YYEOF) {
      break;
    } else {
      ss.intern(yytext);
    }
  }
}

int main(int argc, char **argv) {
    // Setting auxlib execname for debugging.
    set_execname(argv[0]);
    // basename will strip the prepending path.
    const char *execname = basename(argv[0]);
    int exit_status = EXIT_SUCCESS;
    // these are set to 0 because they are not mandatory.
    int err = 0;
    string dflag = "";
    //string atflag = "";

    const char *usage = "usage: oc [-ly] [-@ flag ...] [-D string] program.oc\n";
    // cmd will take on each value of getopt()
    int cmd = 0;
    yy_flex_debug = 0;
    // cmd = -1 when getopt is done parsing the args.
    while ((cmd = getopt(argc, argv, "@:D:ly")) != -1) {
        switch (cmd) {
            case '@':
                //atflag = "-@ " + string(optarg) + " ";
                set_debugflags(optarg);
                break;
            case 'D':
                dflag = " -D" + string(optarg);
                break;
            case 'l':
                yy_flex_debug = 1;
                break;
            case 'y':
                yydebug = 1;
                break;
            case '?':
                // Error in args.
                err = 1;
                break;
            default:
                err = 1;
                break;
        }
    }

    if ((optind + 1) > argc) {
        fprintf(stderr, "%s: missing argument.\n", argv[0]);
        fprintf(stderr, "%s", usage);
        exit(1);
    }
    if (err) {
        fprintf(stderr, "%s: invalid usage.\n", argv[0]);
        fprintf(stderr, "%s", usage);
        exit(1);
    }

    // confirms that .oc is in that string.
    if (strstr(argv[argc - 1], ".oc") != NULL) {
        ocfile = argv[argc - 1];
    } else {
        fprintf(stderr, "%s: no .oc file detected.\n", argv[0]);
        fprintf(stderr, "%s", usage);
        exit(1);
    }

    string command = CPP + dflag + " " + ocfile;


    // Now repeating writing file for .tok   
    string fname_tok = string(basename(ocfile));
    size_t dot_index = fname_tok.find_last_of(".");
    fname_tok = fname_tok.substr(0, dot_index);
    fname_tok = fname_tok + ".tok";

    tok_out = fopen(&fname_tok[0], "w");

    // Opens a file-stream with the output of 'command'
    yyin = popen(command.c_str(), "r");
    if (yyin == NULL) {
        exit_status = EXIT_FAILURE;
        fprintf(stderr, "%s: %s: %s\n", 
                execname, command.c_str(), strerror(errno));
    } else {

        yyparse();
        int pclose_rc = pclose(yyin);
        if (pclose_rc != 0) {
            exit_status = EXIT_FAILURE;
        }
        string fname = string(basename(ocfile));
        fname = fname.substr(0, dot_index);
        string fname_ast = fname + ".ast";
        string fname_sym = fname + ".sym";
        fname = fname + ".str";

        FILE *ofile = fopen(&fname[0], "w");
        FILE *ofile_ast = fopen(&fname_ast[0], "w");
        FILE *ofile_sym = fopen(&fname_sym[0], "w");
        
        ss.dump(ofile);

        symbol_stack* sym_stack_global = new symbol_stack;
        symbol_table* struct_table = new symbol_table;
        sym_stack_global->symbol_stack.push_back(new symbol_table);

        typecheck(ofile_sym, parser::root, sym_stack_global, struct_table);

        // Dump ASTree
        parser::root->print(ofile_ast, parser::root, 0);

        fclose(ofile);
        fclose(tok_out);
        fclose(ofile_ast);
        fclose(ofile_sym);
    }

    return exit_status;
    //exit(exit_status);
}

