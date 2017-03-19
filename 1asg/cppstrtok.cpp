// $Id: cppstrtok.cpp,v 1.6 2016-08-18 13:00:16-07 - - $

// Use cpp to scan a file and print line numbers.
// Print out each input line read in, then strtok it for
// tokens.

#include <string>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <unistd.h>
#include "string_set.h"
#include "auxlib.h"
using namespace std;

const string CPP = "/usr/bin/cpp";
constexpr size_t LINESIZE = 1024;

// Chomp the last character from a buffer if it is delim.
void chomp(char *string, char delim) {
    size_t len = strlen(string);
    if (len == 0) return;
    char *nlpos = string + len - 1;
    if (*nlpos == delim) *nlpos = '\0';
}

// Run cpp against the lines of the file.
// Gives us our individual tokens.
void cpplines(FILE *pipe, char *filename) {
    string_set *ssref = new string_set();
    //testobj->intern("testing");
    int linenr = 1;
    char inputname[LINESIZE];
    // copying from filename to inputname.
    strcpy(inputname, filename);
    // infinite loop
    for (;;) {
        // Declaring a new buffer
        char buffer[LINESIZE];
        // fgets(pointer_to_char_buff, max_num_chars_to_read, FILE_stream_pointer)
        //       returns a pointer to end of the previous call
        char *fgets_rc = fgets(buffer, LINESIZE, pipe);
        if (fgets_rc == NULL) break;

        chomp(buffer, '\n');
        //printf("%s:line %d: [%s]\n", filename, linenr, buffer);
        // http://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html
        int sscanf_rc = sscanf(buffer, "# %d \"%[^\"]\"",
                               &linenr, filename);
        if (sscanf_rc == 2) {
            //printf("DIRECTIVE: line %d file \"%s\"\n", linenr, filename);
            continue;
        }

        //printf("BUFFER: %s\n", buffer);
        char *savepos = NULL;
        char *bufptr = buffer;
        for (int tokenct = 1;; ++tokenct) {
            char *token = strtok_r(bufptr, " \t\n", &savepos);
            printf("token: %s\n", token);
            bufptr = NULL;
            if (token == NULL) break;
            /*printf("token %d.%d: [%s]\n",
                   linenr, tokenct, token);*/
            // Inserting token into our string_set.
            //string_set::intern(token);
            ssref->intern(token);
        }
        ++linenr;
    }

    string fname = string(basename(filename));
    // TODO some kind of error checking for npos perhaps?
    size_t dot_index = fname.find_last_of(".");
    fname = fname.substr(0, dot_index);
    // Adding .str
    fname = fname + ".str";
    //printf("FNAME: %s", &fname[0]);

    FILE *ofile = fopen(&fname[0], "w");
    //string_set::dump(ofile);
    ssref->dump(ofile);
    fclose(ofile);
}

// Usage:  oc [-ly] [-@ flag ...] [-D string] program.oc


int main(int argc, char **argv) {
    // Setting auxlib execname for debugging.
    set_execname(argv[0]);
    // basename will strip the prepending path.
    const char *execname = basename(argv[0]);
    int exit_status = EXIT_SUCCESS;
    // these are set to 0 because they are not mandatory.
    int yy_flex_debug, yydebug, err = 0;
    string dflag = "";
    string atflag= "";

    const char *usage = "usage: oc [-ly] [-@ flag ...] [-D string] program.oc\n";
    // cmd will take on each value of getopt()
    int cmd = 0;
    // cmd = -1 when getopt is done parsing the args.
    while ((cmd = getopt(argc, argv, "@:D:ly")) != -1) {
        switch (cmd) {
            case '@':
                atflag = "-@ " + string(optarg) + " ";
                break;
            case 'D':
                dflag = "-D" + string(optarg) + " ";
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

    if (yy_flex_debug || yydebug || err) {
        // Account for flags here.
    }


    if ((optind + 1) > argc) {
        fprintf(stderr, "%s: missing argument.\n", argv[0]);
        fprintf(stderr, usage);
        exit(1);
    }
    if (err) {
        fprintf(stderr, "%s: invalid usage.\n", argv[0]);
        fprintf(stderr, usage);
        exit(1);
    }

    // Now that I've verified that I have at least one arg, now I need to verify
    // that it's an .oc file.
    string ocfile = string(argv[argc - 1]);
    // exlen = "extension length".
    size_t exlen = ocfile.find_last_of(".");
    // Verifies the arg passed has a . in it.
    // npos is the largest possible int.  Using here to make sure
    // there actually is a "."
    if (exlen < ocfile.npos) {
        /*if (ocfile.substr(exlen) == ".oc") {
            //fprintf(stdout, "oc file %s detected.\n", basename((char*)ocfile.c_str()));
            fprintf(stdout, "oc file %s detected.\n", basename(&ocfile[0]));
        } else {
            fprintf(stderr, "%s: no .oc file detected.\n", argv[0]);
            fprintf(stderr, usage);
            exit(1);
        }*/

        if (ocfile.substr(exlen) != ".oc") {
            fprintf(stderr, "%s: no .oc file detected.\n", argv[0]);
            fprintf(stderr, usage);
            exit(1);
        }

    } else {
        fprintf(stderr, "%s is not a valid file.\n", ocfile.c_str());
        fprintf(stderr, usage);
        exit(1);
    }

    string command = CPP + " " + atflag + dflag + ocfile;
    // Opens a file-stream with the output of 'command'
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe == NULL) {
        exit_status = EXIT_FAILURE;
        fprintf(stderr, "%s: %s: %s\n", execname, command.c_str(), strerror(errno));
    } else {
        cpplines(pipe, (char *) (ocfile.c_str()));
        // pclose() will return -1(? maybe just non-zero) if there's an error.
        int pclose_rc = pclose(pipe);
        // Not doing eprint.
        if (pclose_rc != 0) {
            exit_status = EXIT_FAILURE;
        }
    }

    //return exit_status;
    exit(exit_status);
}

