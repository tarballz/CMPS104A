// Payton Schwarz, peschwar@ucsc.edu

#ifndef __STRING_SET__
#define __STRING_SET__

#include <string>
#include <unordered_set>

using namespace std;

#include <stdio.h>

struct string_set {
    // constructor.
    string_set();

    // set of strings
    static unordered_set <string> set;

    // inserting items into the set
    static const string *intern(const char *);

    // dump the set to a file.
    static void dump(FILE *);
};

#endif

