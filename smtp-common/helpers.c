//
// Created by Никита Иксарица on 23.02.2022.
//

#include "helpers.h"

bool string_ends_with(const char *string, const char *suffix) {
    size_t string_len = strlen(string);
    size_t suffix_len = strlen(suffix);

    return (bool) ((string_len >= suffix_len)
                   && (0 == strcmp(string + (string_len - suffix_len), suffix)));
}

void ensure_trailing_slash(char *string) {
    if (string_ends_with(string, "/") == false) {
        strcat(string, "/");
    }
}
