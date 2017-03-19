// $Id: string_set.h,v 1.2 2016-08-18 15:13:48-07 - - $

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

