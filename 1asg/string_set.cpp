// $Id: string_set.cpp,v 1.4 2016-09-21 16:56:20-07 - - $

#include <string>
#include <unordered_set>

using namespace std;

#include "string_set.h"

unordered_set <string> string_set::set;

string_set::string_set() {
    set.max_load_factor(0.5);
}

const string *string_set::intern(const char *string) {
    // auto will make handle take on the type of whatever set.insert() returns
    auto handle = set.insert(string);
    // & - address of
    // * - dereferenced
    // handle.first - first element in the set
    // even if a duplicate, this func returns the hash for this node.
    return &*handle.first;
}


void string_set::dump(FILE *out) {
    // could also be:  auto hash_fn = ...
    static unordered_set<string>::hasher hash_fn
            = string_set::set.hash_function();
    // size_t is "long long", 64 bit number.
    size_t max_bucket_size = 0;
    for (size_t bucket = 0; bucket < set.bucket_count(); ++bucket) {
        bool need_index = true;
        size_t curr_size = set.bucket_size(bucket);
        if (max_bucket_size < curr_size) max_bucket_size = curr_size;
        // cbegin returns an iterator to the first item in bucket.
        // iterator = pointer.
        // ++itor will then point at the next element in bucket.
        for (auto itor = set.cbegin(bucket);
             itor != set.cend(bucket); ++itor) {
            if (need_index) fprintf(out, "string_set[%4zu]: ", bucket);
            else fprintf(out, "           %4s   ", "");
            need_index = false;
            const string *str = &*itor;
            // %22zu - 22 max chars, long long, unsigned
            // %p address of
            // %s the string
            fprintf(out, "%22zu %p->\"%s\"\n", hash_fn(*str),
                    str, str->c_str());
        }
    }
    fprintf(out, "load_factor = %.3f\n", set.load_factor());
    fprintf(out, "bucket_count = %zu\n", set.bucket_count());
    fprintf(out, "max_bucket_size = %zu\n", max_bucket_size);
}

